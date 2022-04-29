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
    (NodeId, Label, Key, Value)
*/
unique_ptr<BindingIdIter> get_binding_id_iter() {
    auto graph_id = GraphId(0);
    auto& graph = RelationalModel::get_graph(graph_id);

    // Define Scan 1
    // Using B+Tree `label2node` with tuples (LabelId, NodeId) to select all nodes with the label Person
    std::vector<std::pair<VarId, int>> scan1_vars = {
        { VarId(1), 0 }, // VarId(1) => ?n.label
        { VarId(0), 1 }  // VarId(0) => ?n
    };
    auto base = make_unique<TotalScan>(*graph.label2node, scan1_vars);

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


// Antes de probar el test se espera que se cargó una base de datos en la carpeta correcte, ej:
// build/Release/bin/import_graph -d test_files/db -n test_files/graph_creation_example/nodes.txt -e test_files/graph_creation_example/edges.txt
int main() {
    file_manager.init("test_files/db");
    buffer_manager.init();
    RelationalModel::init();

    // std::vector<string> var_names {"?n", "label", "key", "value"};
    std::vector<string> var_names {"?n", "label"};

    auto root = get_binding_id_iter();

    //get first 100 tuples
    uint32_t first_group = 100;
    cout << "fetching first " << first_group << " tuples:" << '\n';

    try {
        std::unique_ptr<BindingId> binding_id = make_unique<BindingId>(var_names.size());
        root->begin(*binding_id);
        BindingId* curr = root->next();
        for(uint32_t iter = 0; iter < first_group && curr != nullptr; iter++){
            print(*curr, var_names);
            curr = root->next();
        }

        // int a;
        // cin >> a;

        //get all tuples
        root->reset(*binding_id);
        cout << "fetching all: " << '\n';
        curr = root->next();
        while (curr != nullptr) {
            print(*curr, var_names);
            curr = root->next();
        }
    }
    catch (std::exception& e) {
        cout << "Exception in query excecution: " << e.what() << endl;
    }
}
