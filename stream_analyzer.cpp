#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <arpa/inet.h>

static void usage ()
{
	std::cout << "总线流水分析工具" << std::endl;
	std::cout << "./stream_analyzer" << std::endl;
}


typedef struct {
	int flow_count;
	int ncount;
	std::vector<int> offset;
	std::vector<int> con_fd;
	std::vector<std::vector<off_t> > idx_offset;
} root_struct_t;

typedef struct 
{
	char IdxPrefix[201];
	char ConPrefix[201];
	int FlowCount; 
	int FlowMaxSize;
	int OpenMax;
	int m_nCount;
} flow_header_t;


static void parse_root (root_struct_t& root_struct)
{
	const char *rootfile = "../store/ft_log_root.id";
	int fd = -1;

	if ((fd = open (rootfile, O_RDONLY)) == -1) {
		perror ("open err");
		exit (EXIT_FAILURE);
	}

	struct stat st;
	assert (fstat (fd, &st) != -1);
	off_t flen = st.st_size;

	flow_header_t flow_header;
	assert (read (fd, (char *)&flow_header,
		sizeof (flow_header_t)) == sizeof (flow_header_t));
	root_struct.flow_count = ntohl (flow_header.FlowCount);
	root_struct.ncount = ntohl (flow_header.m_nCount);
	std::cout << "flow count: " << root_struct.flow_count 
		<< " pkg count: " << root_struct.ncount << std::endl;
	
	char buf[BUFSIZ];
	for (;;) {
		int nread = read (fd, buf, 4);
		if (nread == 0) {
			break;
		}
		assert (nread != -1);

		root_struct.offset.push_back (ntohl (*(int *)buf));
	}	

	close (fd);

	char idx_buf[64 * 1024];
	for (int i = 0; i < root_struct.flow_count; ++i) {
		char buf[512];
		int fd = -1;
		snprintf (buf, 512, "../store/ft_log_%d.id", i + 1);
		assert ((fd = open (buf, O_RDONLY)) != -1);
		
		size_t nread = -1;
		assert ((nread = read (fd, idx_buf, 64 * 1024)) != -1);
		assert (nread % 4 == 0);
		std::vector<off_t> offset_vec;
		for (size_t j = 0; j < nread / 4; ++j) {
			off_t off = ntohl (*(int *)(idx_buf + j * 4));
			offset_vec.push_back (off);
		}
 		root_struct.idx_offset.push_back (offset_vec);
		close (fd);

		snprintf (buf, 512, "../store/ft_log_%d.con", i + 1);
		assert ((fd = open (buf, O_RDONLY)) != -1);
		root_struct.con_fd.push_back (fd);
	}
}

static int show_one_package (off_t position, root_struct_t& root_struct)
{
	int target_con_fd = -1;
	int target_flow = -1;
	if (position > root_struct.ncount) {
		std::cout << "查找的位置: " << position << 
			" 超过范围: " << root_struct.ncount << std::endl;
		return -1;
	}

	assert (root_struct.offset.size () == root_struct.flow_count);
	if (position >= root_struct.offset[root_struct.flow_count - 1]) {
		target_con_fd = root_struct.con_fd[root_struct.flow_count - 1];
		target_flow = root_struct.flow_count;
	} else {
		for (size_t i = 0; i < root_struct.offset.size () - 1; ++i) {
			if (position >= root_struct.offset[i] &&
				position < root_struct.offset[i + 1]) {
				target_con_fd = root_struct.con_fd[i];
				target_flow = i + 1;
			}
		}
	}

	int idx = (position - root_struct.offset[target_flow - 1]) / 100;
	off_t con_offset = root_struct.idx_offset[target_flow - 1][idx];

	assert (lseek (target_con_fd, con_offset, SEEK_SET) != -1);
	
	char con_buf[8192];
	size_t nread = read (target_con_fd, con_buf, 8192);
	assert (nread != -1);
	
	size_t con_size = ntohl (*(int *)con_buf);	
	char *pb = con_buf + 8;
	char *pe = con_buf + 8 + con_size;

	
}

int
main (int argc, char *argv[])
{
	int opt = -1;
	
#if 0
	if (argc == 1) {
		usage ();
		return -1;
	}
#endif
	while ((opt = getopt (argc, argv, "h")) != -1) {
		switch (opt) {
			case 'h':
				usage ();
				break;
		}
	}

	root_struct_t root_struct;
	parse_root (root_struct);
}
