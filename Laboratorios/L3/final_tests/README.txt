Detalles de los tests

Control: Un TotalScan simple (no suma nota, solo controla que la solución compile y que la base de datos esté agregada)
Test 1: BNLJ de dos relaciones, buffer de tamaño 256
Test 2: BNLJ de dos relaciones, buffer de tamaño 64
Test 3: BNLJ de tres relaciones ((A⋈B)⋈C), buffers de tamaño 64
Test 4: BNLJ de cuatro relaciones (D⋈((A⋈B)⋈C)), buffers de tamaño 32, 64, 128
Test 5: imprimir una relación de ~4000 bindings una vez, pasado por un materialize
Test 6: imprimir una relación de ~500 bindings dos veces, pasado por un materialize
Test 7: imprimir una relación de ~4000 bindings dos veces, pasado por un materialize
Test 8: imprimir una relación de ~4000 bindings tres veces, pasado por un materialize
Test 9: ordenar una relacion de ~2000 bindings
Test 10: ordenar una relacion de ~4000 bindings
Test 11: test 2 usando SortMergeJoin
Test 12: test 3 usando SortMergeJoin

Resultados esperados:

Control: 1083
Test 1: 2751 bindings
Test 2: 2751 bindings
Test 3: 2163 bindings
Test 4: 5510 bindings
Test 5: 3970 bindings
Test 6: 542 bindings (dos veces)
Test 7: 3970 bindings (dos veces)
Test 8: 3970 bindings (tres veces)
Test 9: 2002 bindings ordenados por node
Test 10: 3970 bindings ordenados por key, value
Test 11: 2751 bindings
Test 12: 2163 bindings

En los test se revisó que el código implemente los algoritmos pedidos, y que la cantidad de bindings del output sea el correcto. El orden de los bindings no fue considerado excepto en los tests 9 y 10.

Para agregar la base de datos se usó la linea
build/Release/bin/import_graph -d test_files/db -n nodesL3.txt -e edgesL3.txt