#include <iostream>

#include "relational_model/physical_plan/binding_iter/match.h"
#include "relational_model/physical_plan/binding_iter/projection.h"

#include "relational_model/physical_plan/binding_id_iter/total_scan.h"
#include "relational_model/physical_plan/binding_id_iter/sort_merge_join.h"

#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;


unique_ptr<BindingIdIter> get_binding_id_iter() {
    auto graph_id = GraphId(0);
    auto& graph = RelationalModel::get_graph(graph_id);

    std::vector<std::pair<VarId, int>> scan1_vars = {
        { VarId(0), 0 }, // VarId(0) => edge
        { VarId(1), 1 }  // VarId(1) => label
    };
    auto scan1 = make_unique<TotalScan>(*graph.edge2label, scan1_vars);

    std::vector<std::pair<VarId, int>> scan2_vars = {
        { VarId(0), 0 }, // VarId(0) => edge
        { VarId(2), 1 }, // VarId(2) => key
        { VarId(3), 2 }  // VarId(3) => value
    };
    auto scan2 = make_unique<TotalScan>(*graph.edge2prop, scan2_vars);

    return make_unique<SortMergeJoin>(move(scan1), move(scan2));
}


unique_ptr<BindingIter> get_binding_iter() {
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
