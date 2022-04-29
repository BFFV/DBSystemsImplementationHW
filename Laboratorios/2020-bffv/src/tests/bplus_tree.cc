#include <climits>
#include <memory>

#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_params.h"

#include "storage/file_manager.h"
#include "storage/buffer_manager.h"

using namespace std;

void create_ascending(int size, std::string& bpt_name) {
    auto bpt_params =  make_unique<BPlusTreeParams>(bpt_name, 3);
    BPlusTree bpt = BPlusTree(move(bpt_params));

    uint64_t n = 0;
    for (int i = 1; i <= size; i++) {
        uint64_t c[3] = {};
        c[0] = n++;
        c[1] = n++;
        c[2] = n++;

        bpt.insert( Record(c[0], c[1], c[2]) );
    }

    if (!bpt.check()) {
        std::cout << "bpt created with errors\n";
        return;
    }
    std::cout << "bpt created without errors.\n";
}


void create_descending(int size, std::string& bpt_name) {
    auto bpt_params =  make_unique<BPlusTreeParams>(bpt_name, 3);
    BPlusTree bpt = BPlusTree(move(bpt_params));

    uint64_t n = UINT64_MAX;
    for (int i = 1; i <= size; i++) {
        uint64_t c[3] = {};
        c[0] = n--;
        c[1] = n--;
        c[2] = n--;

        bpt.insert( Record(c[0], c[1], c[2]) );
    }

    if (!bpt.check()) {
        std::cout << "bpt created with errors\n";
        return;
    }
    std::cout << "bpt created without errors.\n";
}


void create_random(int size, std::string& bpt_name) {
    auto bpt_params =  make_unique<BPlusTreeParams>(bpt_name, 3);
    BPlusTree bpt = BPlusTree(move(bpt_params));

    for (int i = 1; i <= size; i++) {
        uint64_t c[3] = {};
        c[0] = (uint64_t) rand();
        c[1] = (uint64_t) rand();
        c[2] = (uint64_t) rand();

        bpt.insert( Record(c[0], c[1], c[2]) );
    }

    if (!bpt.check()) {
        std::cout << "bpt created with errors\n";
        return;
    }
    std::cout << "bpt created without errors.\n";
}


int test_order(std::string& bpt_name) {
    auto bpt_params = make_unique<BPlusTreeParams>(bpt_name, 3);
    BPlusTree bpt = BPlusTree(move(bpt_params));

    uint64_t min[] = {0, 0};
	uint64_t max[] = {UINT64_MAX, UINT64_MAX};
	auto it = bpt.get_range(Record(min[0], min[1]), Record(max[0], max[1]));
	auto x = it->next();
	auto y = it->next();
    int i = 1;
    bool error = false;
	while (y != nullptr) {
        i++;
        if (*y <= *x) {
            std::cout << "error en el orden de la tuplas " << (i-1) << " y " << (i) << "\n";
            std::cout << "  (" << x->ids[0] << ", " << x->ids[1] << ", " << x->ids[2] << ")\n  ("
                      << y->ids[0] << ", " << y->ids[1] << ", " << y->ids[2] << ")\n";
            error = true;
        }
		x = std::move(y);
        y = it->next();
	}
    if (!error) {
        std::cout << "Orden de las " << i << " tuplas correcto.\n";
        return 0;
    }
    else {
        return 1;
    }
}

// Ojo al ejecutar 2 veces seguidas el test, dará error por intentar ejecutar keys duplicadas
// deben borrar los archivos creados antes de ejecutarlo de nuevo
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "needed size\n";
        exit(1);
    }
    int size = atoi(argv[1]);
    FileManager::init("test_files/test_bpt");
    BufferManager::init(BufferManager::DEFAULT_BUFFER_POOL_SIZE);

    string name_random = "bpt_random";
    create_random(size, name_random);

    // string name_ascending = "bpt_ascending";
    // create_ascending(size, name_ascending);

    // string name_descending = "bpt_descending";
    // create_descending(size, name_descending);

    // return test_order(name_random) || test_order(name_ascending) || test_order(name_descending);
    return test_order(name_random);
}
