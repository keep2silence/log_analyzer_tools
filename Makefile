all: front_log_analyzer front_log_filter front_login_info

front_log_analyzer: front_log_analyzer.cpp PriceLevel.h
	g++ -o $@ front_log_analyzer.cpp -g -O3

front_log_filter: front_log_filter.cpp
	g++ -o $@ $^ -g -O3
	
front_login_info: front_login_info.cpp
	g++ -o $@ $^ -g -O3 -std=c++0x

debug:
	g++ -o front_log_analyzer front_log_analyzer.cpp -g
	g++ -o front_log_filter front_log_filter.cpp -g
	g++ -o front_login_info front_login_info.cpp -g -std=c++0x
	

clean:
	rm -f front_log_analyzer front_log_filter front_login_info
