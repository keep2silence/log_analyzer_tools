all: front_log_analyzer front_log_filter front_login_info m_ex01 biz_filter_test

front_log_analyzer: front_log_analyzer.cpp PriceLevel.h
	g++ -o $@ front_log_analyzer.cpp -g -O3

front_log_filter: front_log_filter.cpp
	g++ -o $@ $^ -g -O3
	
front_login_info: front_login_info.cpp
	g++ -o $@ $^ -g -O3 -std=c++0x

m_ex01: m_ex01.cpp
	g++ -o $@ $^ -g -std=c++0x -O0

biz_filter_test: biz_filter_test.cpp
	g++ -o $@ $^ -g -std=c++0x -O0

debug:
	g++ -o front_log_analyzer front_log_analyzer.cpp -g
	g++ -o front_log_filter front_log_filter.cpp -g
	g++ -o front_login_info front_login_info.cpp -g -std=c++0x
	

clean:
	rm -f front_log_analyzer front_log_filter front_login_info
