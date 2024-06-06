main:
	g++ -o np_simple np_simple.cpp
	g++ -o np_single_proc np_single_proc.cpp
	g++ -o np_multi_proc np_multi_proc.cpp -pthread
exe1:
	./np_simple 7001
exe2:
	./np_single_proc 7001
exe3:
	./np_multi_proc 7001
clean:
	rm np_simple np_single_proc np_multi_proc