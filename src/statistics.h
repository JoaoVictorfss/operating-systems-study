struct fashion {
  int value;
  int amount;
};

typedef struct fashion Fashion;

void rol(int *list, int low, int high);

// espera os dados ordenados e retorna a moda
Fashion c_fashion(int *list, int size);