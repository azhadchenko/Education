enum State {
    EMPTY = 0,
    READY = 1,
    BUSY = 2
};

struct Ii_element {
    void* item;
    size_t pos_inside;
    struct Ii_element* next;
    size_t state;
};

struct Block {
    size_t count;
    struct Ii_element data[0];
}

struct Ii_manager {
    struct Ii_element* data;

    unsigned char block_size = 0;
    int block_max = 0;
    int block_count = 0;

};

#define INIT_BLOCK_COUNT 16

struct Ii_manager* init_ii_m(){

    size_t block_size = 112;

    if(sizeof(void*) == 4) {
        block_size = 224;
    }

    size_t size = block_size * sizeof(struct Ii_element) + sizeof(struct Block) + sizeof(struct Ii_manager);
    struct Manager* manager = (struct Ii_manager*)calloc(1, size);
    if(!manager)
        return (void*)-1;


    manager -> data = (struct Block**)calloc(sizeof(void*), INIT_BLOCK_COUNT);
    if(!manager -> data) {
        free(manager);
        return (void*)-1;
    }

    manager -> block_size = block_size;
    manager -> data[manager -> manager_count++] = (struct Block*)((void*)manager + sizeof(struct Ii_manager));
    manager -> block_max = INIT_BLOCK_COUNT;

    return manager;
}

ssize_t destruct_manager(struct Ii_manager* manager) {
    if(!manager)
        return 0;

    for(int i = 1; i < manager -> block_count; i++)
        free(manager -> data[i]);

    free(manager);

    return 0;
}

struct Ii_element* allocate_element(struct Ii_manager)* manager){
    if(!manager)
        return 0;

    for(int i = 0; i < manager -> block_count; i++) {

        if(manager -> data[i] -> count == manager -> block_size)
            continue;

        struct Ii_element* tmp = manager -> data[i] -> data;

        for(int j = 0; j < manager -> block_size; j++) {
            if(tmp[j].state == EMPTY) {
                tmp[j].state = BUSY;

                manager -> data[i] -> count++;

                return &tmp[j];
            }

        }
    }

    //actions if all blocks are full

    void* free_candidate = 0;

    if(manager -> block_count == manager -> block_max) {

        struct Block** tmp_data = (struct Block**)calloc(manager -> block_max * 2, sizeof(void*));
        if(!tmp_data)
            return 0;
        memcpy(tmp_data, manager -> data, sizeof(void*) * manager -> block_count);

        free_candidate = manager -> data;
        manager -> data = tmp_data;
        manager -> block_max *= 2;
    }

    size_t index = manager -> block_count;
    struct Block* tmp = (struct Block*)calloc(1, sizeof(struct Block) + sizeof(struct Ii_element) * manager -> block_size);
    if(!tmp)
        return (void*)-1;

    manager -> data[index] = tmp;
    manager -> block_count++;

    free(free_candidate); //Actually this can be a real problem if someone is handling previous pool -> data

    return allocate_element(pool);
}


ssize_t destruct_element(struct Ii_manager* manager, struct Ii_element* item) {
    if(!manager || !item)
        return -1;

    for(int i = 0; i < manager -> spool_count; i++) {
        size_t bottom = (size_t)item - (size_t)(manager -> data[i] -> data);
        size_t top = (size_t)(manager -> data[i] -> data + manager -> spool_size) - (size_t)item;

        if(bottom <= manager -> spool_size * sizeof(struct Ii_element) && top <= manager -> spool_size * sizeof(struct Ii_element)) {
            pool -> data[i] -> count--;
            item -> state = EMPTY;

            return 0;
        }
    }

    return -1;
}


struct Inverted_index{
    struct Ii_manager* manager;
    struct Ii_element* array;
};

struct Inverted_index* init_ii(size_t size) {


}

ssize_t destruct_ii() {

}
