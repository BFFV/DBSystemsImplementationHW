#include <iostream>

#include "relational_model/physical_plan/binding_iter/match.h"
#include "relational_model/physical_plan/binding_iter/projection.h"

#include "relational_model/physical_plan/binding_id_iter/total_scan.h"
#include "relational_model/physical_plan/binding_id_iter/block_nested_loop_join.h"

#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

/*
    manually creates the plan for obtaining all tuples of the form
    (EdgeId, Label, Key, Value)
*/
unique_ptr<BindingIdIter> get_binding_id_iter() {
    auto graph_id = GraphId(0);
    auto& graph = RelationalModel::get_graph(graph_id);

    // Define Scan 1
    // Using B+Tree `label2edge` with all tuples (EdgeId, LabelId)
    std::vector<std::pair<VarId, int>> scan1_vars = {
        { VarId(0), 1 }, // VarId(0) => edge
        { VarId(1), 0 }  // VarId(1) => label
    };
    auto scan1 = make_unique<TotalScan>(*graph.label2edge, scan1_vars);

    // Define Scan 2
    // Using B+Tree `edge2prop` with all tuples (EdgeId, KeyId, ValueId)
    std::vector<std::pair<VarId, int>> scan2_vars = {
        { VarId(0), 0 }, // VarId(0) => edge
        { VarId(2), 1 }, // VarId(2) => key
        { VarId(3), 2 }  // VarId(3) => value
    };
    auto scan2 = make_unique<TotalScan>(*graph.edge2prop, scan2_vars);

    // // return Index Nested Loop Join between scan1 and scan2
    return make_unique<BlockNestedLoopJoin>(move(scan1), move(scan2), 64);
}


unique_ptr<BindingIter> get_binding_iter() {
    // Es necesario al definir var_pos que el minimo VarId es 0 y el máximo es (var_pos.size() - 1)
    std::map<std::string, VarId> var_pos = {
        { "edge", VarId(0) },
        { "label", VarId(1) },
        { "key", VarId(2) },
        { "value", VarId(3) }
    };
    auto match = make_unique<Match>(get_binding_id_iter(), var_pos);

    std::set<std::string> projection_vars {
        "edge",
        "label",
        "key",
        "value"
    };
    return make_unique<Projection>(move(match), projection_vars, 0); // 0 means no LIMIT
}


// Antes de probar el test se espera que se cargó una base de datos en la carpeta correcte, ej:
// build/Release/bin/import_graph -d test_files/db -n test_files/graph_creation_example/nodes.txt -e test_files/graph_creation_example/edges.txt
int main() {
    file_manager.init("test_files/db");
    buffer_manager.init();
    RelationalModel::init();
    auto root = get_binding_iter();

    try {
        root->begin();
        auto binding = root->next();
        auto count = 0;
        while (binding != nullptr) {
            ++count;
            cout << binding->to_string() << '\n';
            binding = root->next();
        }
        cout << "Found " << count << " results.\n";
    }
    catch (std::exception& e) {
        cout << "Exception in query excecution: " << e.what() << endl;
    }
}
