#define RAND_SIZE_USED 8192
typedef int hash_data[4][256];

void hash_init(hash_data a) {
  int i,j;
  for(i=0; i<32; i++)
    for(j=0; j<256; j++)
      a[i][j] = hash_rand();
}

int hash(hash_data a, int key, int mask) {
  int h=0, i;
  for(i=0; i<4; ++i)
    h ^= a[i][((char*)&key)[i]];
  /*printf("[hash=%x]\n", h & mask);*/
  return h & mask;
}
