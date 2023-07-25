#include "graph.h"
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/random.h>
// #include "pbbslib/sample_sort.h"
#include <fstream>
graph::graph(
		const char *fw_beg_file,
		const char *fw_csr_file,
		const char *bw_beg_file,
		const char *bw_csr_file)
{
	double tm=wtime();
	
	typedef int index_tt;
	typedef int vertex_tt;

	vert_count=fsize(fw_beg_file)/sizeof(index_tt) - 1;
	edge_count=fsize(fw_csr_file)/sizeof(vertex_tt);
    
    //fw
	FILE *file=fopen(fw_beg_file, "rb");
	if(file==NULL)
	{
		std::cout<<fw_beg_file<<" cannot open\n";
		exit(-1);
	}

	index_tt *tmp_beg_pos = new index_tt[vert_count+1];
	index_tt ret=fread(tmp_beg_pos, sizeof(index_tt), vert_count+1, file);
	assert(ret==vert_count+1);
	fclose(file);

	file=fopen(fw_csr_file, "rb");
	if(file==NULL)
	{
		std::cout<<fw_csr_file<<" cannot open\n";
		exit(-1);
	}

	vertex_tt *tmp_csr = new vertex_tt[edge_count];
	ret=fread(tmp_csr, sizeof(vertex_tt), edge_count, file);
	assert(ret==edge_count);
	fclose(file);

    //converting to uint32_t
	fw_beg_pos = new index_t[vert_count+1];
	fw_csr = new vertex_t[edge_count];
	
	for(index_t i=0;i<vert_count+1;++i)
		fw_beg_pos[i]=(index_t)tmp_beg_pos[i];

	for(index_t i=0;i<edge_count;++i)
		fw_csr[i]=(vertex_t)tmp_csr[i];

    // bw

	file=fopen(bw_beg_file, "rb");
	if(file==NULL)
	{
		std::cout<<bw_beg_file<<" cannot open\n";
		exit(-1);
	}

	tmp_beg_pos = new index_tt[vert_count+1];
	ret=fread(tmp_beg_pos, sizeof(index_tt), vert_count+1, file);
	assert(ret==vert_count+1);
	fclose(file);

	file=fopen(bw_csr_file, "rb");
	if(file==NULL)
	{
		std::cout<<bw_csr_file<<" cannot open\n";
		exit(-1);
	}

	tmp_csr = new vertex_tt[edge_count];
	ret=fread(tmp_csr, sizeof(vertex_tt), edge_count, file);
	assert(ret==edge_count);
	fclose(file);

    //converting to uint32_t
	bw_beg_pos = new index_t[vert_count+1];
	bw_csr = new vertex_t[edge_count];
	
	for(index_t i=0;i<vert_count+1;++i)
		bw_beg_pos[i]=(index_t)tmp_beg_pos[i];

	for(index_t i=0;i<edge_count;++i)
		bw_csr[i]=(vertex_t)tmp_csr[i];

	delete[] tmp_beg_pos;
	delete[] tmp_csr;

	std::cout<<"Graph load (success): "<<vert_count<<" verts, "
		<<edge_count<<" edges "<<wtime()-tm<<" second(s)\n";
}

graph::graph(const char* filename) {
  //std::cout << "Reading graph...\n";
  double tm=wtime();
  std::ifstream ifs(filename);
  if(!ifs.is_open()) {
    std::cerr << "Error: file " << filename << " does not exist\n";
    exit(EXIT_FAILURE);
  }
  size_t n, m, dummy;
  ifs.read(reinterpret_cast<char*>(&n), sizeof(size_t));
  ifs.read(reinterpret_cast<char*>(&m), sizeof(size_t));
  ifs.read(reinterpret_cast<char*>(&dummy), sizeof(size_t));
  vert_count = n, edge_count = m;

  // forward
  uint64_t *fw_beg_tmp = new uint64_t[n+1];
  uint32_t *fw_csr_tmp = new uint32_t[m];
  ifs.read(reinterpret_cast<char*>(fw_beg_tmp), (n + 1) * sizeof(uint64_t));
  ifs.read(reinterpret_cast<char*>(fw_csr_tmp), m * sizeof(uint32_t));
  if(ifs.peek() != EOF) {
    std::cerr << "Error: Bad data\n";
    exit(EXIT_FAILURE);
  }
  ifs.close();

  fw_beg_pos = new index_t[vert_count+1];
  fw_csr = new vertex_t[edge_count];

  parlay::parallel_for(0, n+1, [&](size_t i) {
    fw_beg_pos[i] = fw_beg_tmp[i];
  });
  parlay::parallel_for(0, m, [&](size_t i) {
    fw_csr[i] = fw_csr_tmp[i];
  });
  delete[] fw_beg_tmp;
  delete[] fw_csr_tmp;

  //std::cout << "Graph loaded: : " << vert_count <<" vertices, "
    //<< edge_count << " edges, " << wtime()-tm <<" second(s)\n";

  //std::cout << "Generating backward edges\n";

  tm = wtime();
  // backward
  bw_beg_pos = new index_t[vert_count+1];
  bw_csr = new vertex_t[edge_count];
  parlay::sequence<std::pair<vertex_t, vertex_t>> edge_list(edge_count);
  
  parlay::parallel_for(0, vert_count, [&](size_t i) {
    parlay::parallel_for(fw_beg_pos[i], fw_beg_pos[i+1], [&](size_t j) {
      edge_list[j] = {fw_csr[j], i};
    });
  });
  //std::cout << "Graph copied\n";

  // sample_sort_inplace(edge_list.slice(), [&](std::pair<vertex_t, vertex_t> a, std::pair<vertex_t, vertex_t> b) {
  //   return a < b;
  // });
  parlay::integer_sort_inplace(edge_list,
																	[&](const std::pair<unsigned, unsigned>& p) { return p.first; });
  //std::cout << "Graph sorted\n";

  parlay::parallel_for(0, edge_list[0].first, [&](size_t i) {
    bw_beg_pos[i] = 0;
  });
  parlay::parallel_for(0, edge_count, [&](size_t i) {
    vertex_t u = edge_list[i].first;
    vertex_t v = edge_list[i].second;
    bw_csr[i] = v;
    if(i == 0 || (edge_list[i-1].first != u)) {
      bw_beg_pos[u] = i;
    }
    if(i == edge_count-1 || (edge_list[i+1].first != u)) {
      vertex_t end = (i==edge_count-1?vert_count+1:edge_list[i+1].first);
      parlay::parallel_for(u+1, end, [&](size_t j) {
        bw_beg_pos[j] = i+1;
      });
    }
  });
  parlay::parallel_for(0, n+1, [&](size_t i) {
    assert(i == n || bw_beg_pos[i] <= bw_beg_pos[i+1]);
    assert(bw_beg_pos[i] <= m);
  });
  parlay::parallel_for(0, m, [&](size_t i) {
    assert(bw_csr[i] >= 0 && bw_csr[i] < n);
  });
  //std::cout << "Backward edges generated, " << wtime()-tm << " second(s)\n";
}
