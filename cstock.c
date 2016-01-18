
/**
 SIMPLE STOCK CONTROL
 Author: Leandro Israel Pinto
 www.leandroip.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * O arquivo de dados é dividido em slots. Cada slot pode estar ocupado
 * (STATUS_BUSY) ou livre (STATUS_EMPTY). 
 */
#define STATUS_EMPTY 0
#define STATUS_BUSY  1

/* Nome do arquivo de dados */
#define DATABASE_FILENAME "database.dat"

/* Quantidade máxima de slots, ou de itens no estoque.*/
#define DATABASE_MAX 1000

/* 
 * Estrutura para organizar os dados no arquivo. 
 * Se alterar esse arquivo, o arquivo de dados não será mais compativel.
 * O arquivo de dados é criado automaticamente, se o mesmo não existir.
 */
struct st_product {
    char status;
    char description[500];
    int qtty;
    float price;
};

typedef struct st_product PRODUCT;

/** Ponteiro para o arquivo de dados. */
FILE *database_file;

/* Limpar a tela */
void clear_screen() {
    int i;
    for (i = 0; i < 85; i++) {
        printf("\n");
    }
}

/* Aguardar alguma entrada do usuário. */
void press_any_key() {
    char a[10];
    printf("Press ENTER to continue...");
    //scanf("%s", a);
    fgetc(stdin);

}

/* 
 * Imprime uma linha da tabela de estoque.  
 */
void print_row(char *id, char *desc, char *qtty, char *price) {
    printf("%5s %-45s %6s %6s\n", id, desc, qtty, price);
}

/* 
 * Imprime o cabeçalho da tabela de estoque. 
 */
void print_header() {
    print_row("ID", "DESCRIPTION", "QTTY", "PRICE");
}

/* 
 * Imprime um produto na tabela. 
 */
void print_product_row(PRODUCT *p, int idp) {
    char id[10], qtty[10], price[10];

    sprintf(id, "%d", idp);
    sprintf(qtty, "%d", p->qtty);
    sprintf(price, "%.2f", p->price);

    //clear_screen();
    //print_header();
    print_row(id, p->description, qtty, price);
}

/* 
 * Preapara o arquivo de dados. Se o mesmo não existir, essa função 
 * criará um com capacidade para DATABASE_MAX itens.
 */
void prepare_file() {
    int i;
    PRODUCT p;
    p.status = STATUS_EMPTY;
    p.description[0] = '\0';

    if (database_file != NULL) {
        fclose(database_file);
        database_file = fopen(DATABASE_FILENAME, "r+b");
        return;
    }

    database_file = fopen(DATABASE_FILENAME, "r+b");
    if (!database_file) {
        printf("Database do not exists.\nCreating...");
        database_file = fopen(DATABASE_FILENAME, "wb");

        for (i = 0; i < DATABASE_MAX; i++) {
            fseek(database_file, i * sizeof (PRODUCT), SEEK_SET);
            fwrite(&p, sizeof (PRODUCT), 1, database_file);
        }

        fclose(database_file);
        database_file = fopen(DATABASE_FILENAME, "r+b");

        printf("Success\n\n");
        press_any_key();
    } else {
        printf("Database file OK\n\n");
    }
}

/* 
 * Localiza um slot livre no arquivo de dados.
 */
int db_get_free_position() {
    int pos;
    PRODUCT p;

    pos = 0;
    fseek(database_file, pos * sizeof (PRODUCT), SEEK_SET);
    fread(&p, sizeof (PRODUCT), 1, database_file);
    while (p.status == STATUS_BUSY) {
        pos++;
        if (pos >= DATABASE_MAX) return -1;
        fseek(database_file, pos * sizeof (PRODUCT), SEEK_SET);
        fread(&p, sizeof (PRODUCT), 1, database_file);
    }
    if (pos >= DATABASE_MAX) return -1;
    return pos;
}

/*
 * Salva um produto no arquivo de dados no slot 'id'. 
 * Se 'id' for -1, localiza um slot livre.
 * Se 'id' for maior ou igual a 0, substitui o slot existente por 'p'.
 * Retorna o 'id' utilizado, ou posição do slot utilizado.
 */
int save_product(PRODUCT *p, int id) {
    int i, idp;
    int pos;
    idp = id;
    //p->status = STATUS_BUSY;
    if (id == -1) {
        pos = db_get_free_position();
        if (pos == -1) {
            printf("Error: No more slots!\n\n");
            press_any_key();
            return -1;
        }
        idp = pos;
    }
    fseek(database_file, idp * sizeof (PRODUCT), SEEK_SET);
    fwrite(p, sizeof (PRODUCT), 1, database_file);
    return idp;
}

/*
 * Retorna um produto em 'p' que está no slot 'id'.
 * Retorna 1 para sucesso e -1 em caso de falha. 
 */
int get_product_by_id(PRODUCT *p, int id) {
    fseek(database_file, id * sizeof (PRODUCT), SEEK_SET);
    fread(p, sizeof (PRODUCT), 1, database_file);
    if (p->status == STATUS_BUSY) return 1;
    return -1;
}

/*
 * Insere um novo produto. 
 */
void stock_new() {
    PRODUCT np;
    int id;
    char str[50];
    clear_screen();
    np.status = STATUS_BUSY;
    printf("NEW PRODUCT\n\n");

    printf("Description: ");
    fgets(np.description, 500, stdin);
    if (strlen(np.description) > 0) {
        np.description[strlen(np.description) - 1] = '\0';
    }

    printf("Initial quantity: ");
    fgets(str, 50, stdin);
    np.qtty = atoi(str);

    printf("Price: ");
    fgets(str, 50, stdin);
    np.price = atof(str);


    id = save_product(&np, -1);
    clear_screen();
    print_header();
    print_product_row(&np, id);
    press_any_key();
}

/*
 * Lista os produtos.
 */
void stock_list() {
    int i;
    PRODUCT p;
    clear_screen();
    print_header();
    for (i = 0; i < DATABASE_MAX; i++) {
        fseek(database_file, i * sizeof (PRODUCT), SEEK_SET);
        fread(&p, sizeof (PRODUCT), 1, database_file);
        if (p.status == STATUS_BUSY) {
            print_product_row(&p, i);
        }
    }
    press_any_key();
}

/*
 * Edita um produto.
 */
void stock_edit() {
    PRODUCT p;
    int id, o;
    char s[500];
    clear_screen();
    printf("Product id: ");
    fgets(s, 20, stdin);
    id = atoi(s);

    if (get_product_by_id(&p, id) == 1) {
        o = 100;
        while (o != 0) {
            clear_screen();
            printf("EDITING PRODUCT\n");
            print_header();
            print_product_row(&p, id);
            printf("\n\n");
            printf(" 1. Edit description.\n");
            printf(" 2. Edit quantity.\n");
            printf(" 3. Edit Price.\n\n");
            printf(" 0. Return.\n\n");
            printf("Option: ");
            fgets(s, 20, stdin);
            o = atoi(s);
            switch (o) {
                case 1:
                    printf("New description: ");
                    fgets(p.description, sizeof (p.description), stdin);
                    if (strlen(p.description) > 0) {
                        p.description[strlen(p.description) - 1] = '\0';
                    }
                    save_product(&p, id);
                    break;
                case 2:
                    printf("New quantity: ");
                    fgets(s, 50, stdin);
                    p.qtty = atoi(s);
                    save_product(&p, id);
                    break;
                case 3:
                    printf("New price: ");
                    fgets(s, 50, stdin);
                    p.price = atof(s);
                    save_product(&p, id);
                    break;
            }
        }
        printf("Succes editing.\n");
        press_any_key();
    } else {
        printf("Error, product not found!\n");
        press_any_key();
    }

}

/*
 * Exclui um produto.
 */
void stock_delete() {
    char s[20];
    int id;
    PRODUCT p;
    clear_screen();
    printf("Insert product ID to delete: ");
    fgets(s, 20, stdin);
    id = atoi(s);
    if (get_product_by_id(&p, id) == 1) {
        printf("\n\n\n");
        print_header();
        print_product_row(&p, id);
        printf("\n\n");
        printf("Are you sure to delete this product?\nType y(yes) or n(no): ");
        fgets(s, 20, stdin);
        if (s[0] == 'y') {
            p.status = STATUS_EMPTY;
            save_product(&p, id);
            printf("Success deleting product.\n");
            press_any_key();
        }

    } else {
        printf("Error, can't found product!\n");
        press_any_key();
    }

}

/*
 * Mostra o menu principal e obtem uma entrada do usuário.
 */
int get_menu_main() {
    char s[20];

    clear_screen();
    printf("----- STOCK CONTROL -----\n");
    printf(" 1. List\n");
    printf(" 2. New\n");
    printf(" 3. Edit\n");
    printf(" 4. Delete\n\n");
    printf(" q. Quit\n");
    printf("\n\n");
    printf("Choose option: ");
    fgets(s, 20, stdin);

    return s[0];
}

/*
 * Executa as ações do usuário na tela inicial.
 */
int main(int argc, char **argv) {
    int option;
    option = '0';

    while (option != 'q') {
        clear_screen();
        prepare_file();
        option = get_menu_main();
        switch (option) {
            case '1':
                stock_list();
                break;
            case '2':
                stock_new();
                break;
            case '3':
                stock_edit();
                break;
            case '4':
                stock_delete();
                break;
            case 'q':
                break;
            default:
                printf("Invalid Option!");
                press_any_key();
                break;
        }
    }
    return 0;
}


