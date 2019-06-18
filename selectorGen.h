#pragma once

class SelectorGen
{

 public:

  int num_selectors;
    
  struct record {
    uint lst;
    uint hst;
    uint hxp;
    uint md;
  };

  record rc;
  
 public:
  SelectorGen(int nselectors, uint lowest, uint highest, uint highexp, uint mode)
    {
      num_selectors = nselectors;
      rc.lst = lowest;
      rc.hst = highest;
      rc.hxp = highexp;
      rc.md = mode;
    }

  
 public:

  void print_table(int **table);
  void generate(int **dest);
  

};
