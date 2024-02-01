#include<stdio.h>
#include<stdlib.h>
#include<string.h>
const char privateCode[] =  "20205057";
enum accountStatus {ACTIVE = 1, BLOCKED =0, IDLE =2};

typedef struct User
{
    char username[100];
    char password[100];
    int  status;

} User;

typedef struct Node {
    User user;
    int isOnline;
    struct Node *pNext;
} node;
void showList(node *root)
{
    while(root!=NULL)
    {
        printf("%-50s %-30s\n",root->user.username,root->user.password);
        root=root->pNext;
    }

}
node *push (node *root, User user) {
    node *new = (node*)malloc(sizeof(node));
    new -> user = user;
    new ->pNext = NULL;
    if(root==NULL) { 
        root = new;
    } else{
        new -> pNext = root;
        root = new;
    }

    
    return root;
}

node *readData(node *root)
{
    FILE *file;
    User temp;
    file =fopen("nguoidung.txt","r");
    if(file!=NULL)
    {
        while(fscanf(file,"%s %s %d",temp.username, temp.password,&temp.status) != EOF)
        {
            root = push(root,temp);
        }
    }
    // showList(list);
    fclose(file);
    return root;
}


void saveData(node *root){
    FILE *file;
    file =fopen("nguoidung.txt","wt");
    while (root!=NULL)
    {
        fprintf(file,"%s %s %d\n", root->user.username,root->user.password,root->user.status);
        root = root->pNext;
    }
    fclose(file);   
}


void emptyList(node *first) {
    node *tmp;
    while(first != NULL) {
        tmp = first;
        first = first ->pNext;
        free(tmp);
    }
}


int checkCharactor(char *str) {
    for (int i = 0;i< strlen(str) ;i++) {
        if(str[i] == ' ') {
            return 0;
            break;
        }
    }
    return 1;
}

int checkUsername (node *root ,char *username) {
    
    while (root != NULL) 
    {
        if(strcmp(root->user.username, username) == 0) {
            return 0;
            break;
        }
        root = root->pNext;
    }
    return 1;
}

node *signup(node *root ) {
    fflush(stdin);
    char username[50];
    char password[50];
    printf("__________Reister__________\n");
    printf("Username: ");gets(username);
    if(!checkUsername(root, username)) {
        printf("Account existed\n");
        return root;
    }
    if (!checkCharactor(username) ) {
        printf("Username cannot contain spaces.\n");
        return root;
    }
    if(checkUsername(root, username) && checkCharactor(username)) {
        printf("Password: ");gets(password);
        if (!checkCharactor(password) ) {
            printf("Password cannot contain spaces.\n");
            return root;
        } else {
            User newUser;
            strcpy(newUser.username, username);
            strcpy(newUser.password, password);
            newUser.status = IDLE;
            root = push(root, newUser);
            saveData(root);
            printf("Successful registration. Activation required.\n");
        }
    }
    return root;
    
} 

void activeAccount(node *root) {
    char username[50],password[50], code[8];
    int checkUsername = 0 ,checkCode = 0;
    node *currentUser =  root;
    printf("__________Login__________\n");
    printf("Username: ");scanf("%s",username);
    printf("%s :))", username);
    while(currentUser!=NULL) {
        if(strcmp(currentUser->user.username, username)==0) {
            checkUsername = 1;
            break;
        }
        currentUser = currentUser->pNext;
    }
    if(checkUsername!=1) {
        printf("Cannot find account\n");
    } else {
       printf("Password: ");scanf("%s",password);
       if(strcmp(password,currentUser->user.password) != 0) {
        printf("Password is incorrect\n");
       } else {
        do {
            printf("Code: "); scanf("%s", code);
            if (strcmp(code, privateCode) !=0) {
                if(checkCode == 4) {
                    currentUser->user.status = BLOCKED;
                    saveData(root);
                    printf("Activation code is incorrect. Account is blocked\n");
                    break;
                }
                printf("Account is not activated\n");
            } else {
                currentUser->user.status = ACTIVE;
                saveData(root);
                printf("Account is activated\n");
                break;
            }
        }while(checkCode < 5);}
    }
}

void login(node *root) {
    char username[50], password[50];
    int checkUsername = 0 ,checkPassword = 0;
    node *currentUser =  root;
    printf("__________Login__________\n");
    printf("Username: ");scanf("%s",username);
    while(currentUser!=NULL) {
        if(strcmp(currentUser->user.username, username)==0) {
            checkUsername = 1;
            break;
        }
        currentUser = currentUser->pNext;
    }
    if(checkUsername != 1) {
        printf("Cannot find account\n");
    } else {
        if(currentUser->user.status == BLOCKED) {
            printf("Account is blocked\n");
        } else {
            do {
                printf("\nPassword: ");scanf("%s",password);
                if(strcmp(password, currentUser->user.password) != 0) {
                    if(checkPassword == 3) {
                        currentUser->user.status = BLOCKED;
                        saveData(root);
                        printf("Activation code is incorrect.Account is blocked\n");
                    }
                    checkPassword++;
                    printf("Password is incorrect\n");
                } else {
                    printf("Hello %s\n", username);
                    currentUser->isOnline = 1;
                    break;
                }
            } while (checkPassword < 4);
        }}
}

void searchAccount(node *root) {
    char username[50];
    int isFound = 0;
    node *result = root;
    printf("__________Search user__________\n");
    printf("Username: "); scanf("%s", username);
    while(result != NULL) {
        if(strcmp(result->user.username, username)==0) {
            isFound = 1;
            break;
        }
        result = result->pNext;
    }
    if(isFound != 1) {printf("Cannot find account\n");}
    else {
        if(result->user.status == ACTIVE) {printf("Account is active\n");}
        else if(result->user.status == BLOCKED) {printf("Account is blocked\n");}
        else {printf("Account is not active\n");}
    }
    

}
void changePassword(node *root) {
    char username[50], password[50], newPassword[50];
    int isFound = 0;
    node *currentAccount = root;
    printf("__________Change password__________\n");
    printf("Username: "); scanf("%s", username);
    while(currentAccount != NULL) {
        if(strcmp(currentAccount->user.username, username)==0) {
            isFound = 1;
            break;
        }
        currentAccount = currentAccount->pNext;
    }
    if(isFound == 0) {
        printf("Cannot find account\n");
    } else {
        if(currentAccount->isOnline != 1) {
            printf("Account is not sign in\n");
        } else {
            printf("Password: "); scanf("%s", password);
            if(strcmp(currentAccount->user.password, password) !=0) {
                printf("Current password is incorrect. Please try again\n");
            } else {
                printf("New password: "); scanf("%s", newPassword);
                strcpy(currentAccount->user.password, newPassword);
                saveData(root);
                printf("Password is changed\n");
            }
        }
    }

}

void logout(node *root) {
    char username[50];
    int isFound = 0;
    node *currentAccount = root;
    printf("__________Logout__________\n");
    printf("Username: "); scanf("%s", username);
    while(currentAccount != NULL) {
        if(strcmp(currentAccount->user.username, username)==0) {
            isFound = 1;
            break;
        }
        currentAccount = currentAccount->pNext;
    }
    if(isFound ==0 ) {printf("Cannot find account\n");}
    else {
        if(currentAccount->isOnline != 1) {
            printf("Account is not sign in\n");
        } else {
            printf("Goodbye %s\n", username);
            currentAccount->isOnline = 0;
        }
    }
}


int main() {
    node *root = NULL;
    int choose;
    root = readData(root);

    // printf("%s", root->user.username);
    // printf("++===========");
    // showList(root);
    do {
        printf("\nUSER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Activate\n");
        printf("3. Sign in\n");
        printf("4. Search\n");
        printf("5. Change password\n");
        printf("6. Sign out\n");
        printf("\n\n\nYour choice (1-6, other to quit): ");
        scanf("%d", &choose);
        switch (choose)
        {
        case 1:
            root = signup(root);
            break;
        case 2:
            activeAccount(root);
            break;
        case 3:
            login(root);
            break;
        case 4: 
            searchAccount(root);
            break;
        case 5:
            changePassword(root);
            break;
        case 6:
            logout(root);
            break;
        default:
            emptyList(root);
            exit(0);
        } 
    } while(1);
}