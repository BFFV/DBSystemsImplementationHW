#include <iostream>

#include "relational_model/physical_plan/binding_iter/match.h"
#include "relational_model/physical_plan/binding_iter/projection.h"

#include "relational_model/physical_plan/binding_id_iter/total_scan.h"
#include "relational_model/physical_plan/binding_id_iter/external_merge_sort.h"

#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

//get node properties sorted by nodeid, sort by key, value
unique_ptr<BindingIdIter> get_binding_id_iter() {
    auto graph_id = GraphId(0);
    auto& graph = RelationalModel::get_graph(graph_id);

    std::vector<std::pair<VarId, int>> scan_vars = {
        { VarId(2), 0 }, // VarId(1) => node
        { VarId(0), 1 }, // VarId(0) => key
        { VarId(1), 2 } // VarId(2) => value
    };
    auto scan = make_unique<TotalScan>(*graph.node2prop, scan_vars);

    return make_unique<ExternalMergeSort>(move(scan));
}


unique_ptr<BindingIter> get_binding_iter() {
    std::map<std::string, VarId> var_pos = {
        { "key", VarId(0) },
        { "value", VarId(1) },
        { "node", VarId(2) }
    };
    auto match = make_unique<Match>(get_binding_id_iter(), var_pos);

    std::set<std::string> projection_vars {
        "key",
        "value",
        "node"
    };
    return make_unique<Projection>(move(match), projection_vars, 0); 
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
