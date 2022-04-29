#include <iostream>

#include "relational_model/physical_plan/binding_iter/match.h"
#include "relational_model/physical_plan/binding_iter/projection.h"

#include "relational_model/physical_plan/binding_id_iter/total_scan.h"
#include "relational_model/physical_plan/binding_id_iter/materialize.h"

#include "relational_model/binding/binding_match.h"
#include "relational_model/binding/binding_project.h"

#include "relational_model/graph/relational_graph.h"
#include "relational_model/relational_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

/*
    manually creates the plan for obtaining all tuples of the form
    (NodeId, Key, Value)
*/
unique_ptr<BindingIdIter> get_binding_id_iter() {
    auto graph_id = GraphId(0);
    auto& graph = RelationalModel::get_graph(graph_id);

    // Define Scan
    // Using B+Tree `node2prop` with tuples (LabelId, NodeId) to select all nodes with the label Person
    std::vector<std::pair<VarId, int>> scan_vars = {
        { VarId(0), 0 }, // VarId(0) => node
        { VarId(1), 1 }, // VarId(1) => key
        { VarId(2), 2 }  // VarId(2) => value
    };
    auto base = make_unique<TotalScan>(*graph.node2prop, scan_vars);

    return make_unique<Materialize>(move(base));
}

void print(BindingId binding_id, std::vector<string> var_names){
    cout << "{";
    for(uint32_t i = 0; i < var_names.size(); i++){
        cout << var_names[i] << ":" << RelationalModel::get_graph_object(binding_id[i])->to_string();
        if(i == var_names.size()-1){
            cout << "}";
        }
        else{
            cout << ",";
        }
    }
    cout << '\n';
}


// Antes de probar el test se carga la base de datos del lab
// build/Release/bin/import_graph -d test_files/db -n nodesL3.txt -e edgesL3.txt
int main(int argc, char *argv[]) {
    file_manager.init("test_files/db");
    buffer_manager.init();
    RelationalModel::init();

    // std::vector<string> var_names {"?n", "label", "key", "value"};
    std::vector<string> var_names {"node", "key", "value"};

    auto root = get_binding_id_iter();


    try {
        if(argc == 1){
            int a;
            cin >> a;
        }

        //get all tuples
        std::unique_ptr<BindingId> binding_id = make_unique<BindingId>(var_names.size());
        root->begin(*binding_id);
        BindingId* curr = root->next();
        cout << "fetching all: " << '\n';
        int count = 0;
        while (curr != nullptr) {
            count++;
            print(*curr, var_names);
            curr = root->next();
        }
        cout << "Found " << count << " results.\n";
    }
    catch (std::exception& e) {
        cout << "Exception in query excecution: " << e.what() << endl;
    }
}
