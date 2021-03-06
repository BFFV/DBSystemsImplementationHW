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
    // Using B+Tree `edge2label` with all tuples (NodeId, LabelId)
    std::vector<std::pair<VarId, int>> scan1_vars = {
        { VarId(0), 0 }, // VarId(0) => node_from
        { VarId(1), 1 }  // VarId(1) => label1
    };
    auto scan1 = make_unique<TotalScan>(*graph.node2label, scan1_vars);

    // Define Scan 2
    // Using B+Tree `edge_from_to` with all tuples (EdgeId, NodeId, NodeId)
    std::vector<std::pair<VarId, int>> scan2_vars = {
        { VarId(2), 0 }, // VarId(2) => edge
        { VarId(0), 1 }, // VarId(0) => node_from
        { VarId(5), 2 }  // VarId(5) => node_to
    };
    auto scan2 = make_unique<TotalScan>(*graph.edge_from_to, scan2_vars);

    auto join1 = make_unique<BlockNestedLoopJoin>(move(scan1), move(scan2), 32); 

    // Define Scan 3
    // Using B+Tree `edge2prop` with all tuples (EdgeId, KeyId, ValueId)
    std::vector<std::pair<VarId, int>> scan3_vars = {
        { VarId(2), 0 }, // VarId(2) => edge
        { VarId(3), 1 }, // VarId(3) => edge_key
        { VarId(4), 2 }  // VarId(4) => edge_value
    };
    auto scan3 = make_unique<TotalScan>(*graph.edge2prop, scan3_vars);

    auto join2 = make_unique<BlockNestedLoopJoin>(move(join1), move(scan3), 64); 

    // Define Scan 4
    // Using B+Tree `node2label` with all tuples (NodeId, LabelId)
    std::vector<std::pair<VarId, int>> scan4_vars = {
        { VarId(5), 0 }, // VarId(5) => node_to
        { VarId(6), 1 }  // VarId(6) => label2
    };
    auto scan4 = make_unique<TotalScan>(*graph.node2label, scan4_vars);

    // // return Index Nested Loop Join between scan1 and scan2
    return make_unique<BlockNestedLoopJoin>(move(scan4), move(join2), 128); 
}


unique_ptr<BindingIter> get_binding_iter() {
    // Es necesario al definir var_pos que el minimo VarId es 0 y el m??ximo es (var_pos.size() - 1)
    std::map<std::string, VarId> var_pos = {
        { "node_from", VarId(0) },
        { "label1", VarId(1) },
        { "edge", VarId(2) },
        { "edge_key", VarId(3) },
        { "edge_value", VarId(4) },
        { "node_to", VarId(5) },
        { "label2", VarId(6) }
    };
    auto match = make_unique<Match>(get_binding_id_iter(), var_pos);

    std::set<std::string> projection_vars {
        "node_from",
        "label1",
        "edge",
        "edge_key",
        "edge_value",
        "node_to",
        "label2"
    };
    return make_unique<Projection>(move(match), projection_vars, 0); // 0 means no LIMIT
}


// Antes de probar el test se espera que se carg?? una base de datos en la carpeta correcte, ej:
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
