void write_example_mask() {

  for(int row = 0; row < 16; row++) {
    for(int col = 0; col < 64; col++) {

      int mask = 0;

      /* example: some diagonals
      int mask = 0;
      if(row==col) mask = 1;      // diagonal in column 0 - 15
      if(row+16==col+1) mask = 1; // shifted diagonal
      if(col>=32) mask = 1;       // invert on -> off
      if(row+32==col) mask = 0;   // diagonal in column 16 - 31
      if(row+48==col+1) mask = 0; // diagonal in column 16 - 31
      //std::cout << mask << " ";
      */

      std::cout << col << " " << row << " 1 " << mask << " 4 0" << std::endl;
    }
    // std::cout << std::endl;
  }

  return;
}
