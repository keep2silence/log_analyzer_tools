all: front_log_analyzer front_log_filter

front_log_analyzer: front_log_analyzer.cpp PriceLevel.h
	g++ -o $@ front_log_analyzer.cpp -g -O3

front_log_filter: front_log_filter.cpp
	g++ -o $@ $^ -g -O3
	
clean:
	rm -f front_log_analyzer front_log_filter
