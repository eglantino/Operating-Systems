CXX = g++
CXXFLAGS = -std=c++11

# Define the list of targets
TARGETS = Sorter MergeSort Merger mysort QuickSort

all: $(TARGETS)

Sorter: Sorters.cpp utils.o
	$(CXX) $(CXXFLAGS) -o Sorter Sorters.cpp utils.o

MergeSort: MergeSort.cpp utils.o
	$(CXX) $(CXXFLAGS) -o MergeSort MergeSort.cpp utils.o

QuickSort: QuickSort.cpp utils.o
	$(CXX) $(CXXFLAGS) -o QuickSort QuickSort.cpp utils.o

Merger: Mergers.cpp utils.o
	$(CXX) $(CXXFLAGS) -o Merger Mergers.cpp utils.o

mysort: Coordinator.cpp utils.o
	$(CXX) $(CXXFLAGS) -o mysort Coordinator.cpp utils.o

utils.o: utils.cpp
	$(CXX) $(CXXFLAGS) -c -o utils.o utils.cpp

clean:
	rm -f $(TARGETS) utils.o