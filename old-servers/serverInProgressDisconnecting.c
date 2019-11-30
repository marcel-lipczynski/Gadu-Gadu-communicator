#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>




#define SERVER_PORT 1234
#define QUEUE_SIZE 10
#define LICZBA_KLIENTOW 10 // maksymalna ilość klientów - póki co 3
#define LOGIN_LENGTH 100 // długość loginu użytkownika -> max 100 znaków
#define MAX_MESSAGE_LENGTH 1000;

pthread_mutex_t lock; // przy odbieraniu wiadomosci od kilku na raz

// int clients[3];
// char names[3][100]; // trzech userów każdy moze miec nazwe po 100 znakow
// int ile;


//structura przechowujaca login i deskryptor uzytkownika
struct user{
	int deskryptor_klienta;
	int dlugosc_loginu;
	char login[100]; // maksymalnie 100 znakow
};


//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
	int socket_descriptor;
	char bufor[1000];
	char tmpBufor[1000];
	//do kogo wyslac wiadomosc?
	char *whoToSend;
	// char **pointer_loginy; // wskaznik na tablice loginow
	// int *pointer_klienci; // wskaznik na tablice klientow
	struct user *ptr_all_users;

};

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data)
{
    //message length as a char
    char message_long[4];
    int result;
	char exitSeq[] = "#!#";
	//zmienna dlugosc wiadomosci
	int message_length;
	//zmienna przechowujaca liczbe przeczytanych bajtow
	int readed_bytes = 0;
	//liczba wyslanych bajtow
	int written_bytes = 0;
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    
	
while(1){

	for(int i = 0; i<LICZBA_KLIENTOW; i++){
			printf("Loginy uzytkownikow %s\n", th_data->ptr_all_users[i].login);
		}

	//czysc oba bufory!
	memset(th_data->bufor, 0, sizeof(th_data->bufor));
	memset(th_data->tmpBufor, 0, sizeof(th_data->tmpBufor));






    while(readed_bytes < 4){
        result = read(th_data->socket_descriptor, message_long + readed_bytes, sizeof(message_long));
        if(result == -1){
            printf("Blad czytania we ThreadBehavior");
        }
        readed_bytes += result;
    }

    message_length = (message_long[3] << 24 | message_long[2] << 16 | message_long[1] << 8 | message_long[0]);

    printf("Dlugosc wiadomosci po przesunieciu bitowym to %d", message_length);


	
	// //odczytanie długości wiadomości która zostanie wysłana
	// while((readed_bytes=read(th_data->socket_descriptor, &message_length, sizeof(int)) < 1)){
	// 	if(readed_bytes == -1){
	// 		printf("Blad podczas odczytywania dlugosci wiadomosci");
	// 	}
	// }


    // message_length = htonl(message_length);
	printf("Przeczytane bajty: %d", readed_bytes);
	printf("Dlugosc nadchodzacej wiadomosci to %d\n", message_length);

	
	while((readed_bytes=read(th_data->socket_descriptor,th_data->bufor,message_length)<message_length)){
		if(readed_bytes == -1){
			printf("Blad podczas odczytywania przeslanej wiadomosci");
			exit(0);
		}
	}


	printf("%s\n",th_data->bufor);
	strncpy(th_data->tmpBufor,th_data->bufor,message_length);

	th_data->whoToSend = strtok(th_data->bufor,":");
	printf("first word is %s\n",th_data->whoToSend);
	th_data->whoToSend = strtok(NULL,":");
	printf("second word is %s\n",th_data->whoToSend);


    //sprawdzanie czy uzytkownik nie wyslal sekwencji wyjsciowej #!# jesli tak to informuje innych uzytkownikow
    //i konczy dzialanie
    if(strcmp(th_data->whoToSend,exitSeq)==0){
        printf("End thread! USER LOG OUT");

        for(int i=0; i<LICZBA_KLIENTOW; i++){
            if((th_data->ptr_all_users[i].deskryptor_klienta != 0) & (th_data->ptr_all_users[i].deskryptor_klienta != th_data->socket_descriptor)){
                if(write(th_data->ptr_all_users[i].deskryptor_klienta,"#!#\n",4) == -1){
                    printf("Error przy wysylaniu informacji o wylogowaniu uzytkownika");
                }
                if(write(th_data->ptr_all_users[i].deskryptor_klienta, th_data->tmpBufor,message_length)==-1){
                    printf("Error przy wysylaniu wiadomosci ktora wyslal uzytkownik");
                }
				//jesli to odlaczajacy sie user to wpisz tam zero :)
				if(th_data->ptr_all_users[i].deskryptor_klienta == th_data->socket_descriptor){
					th_data->ptr_all_users[i].deskryptor_klienta = 0;
				}
            }
        }
        // if(th_data->ptr_all_users[i].deskryptor_klienta != th_data->socket_descriptor


        free(th_data);
        pthread_exit(NULL);
    }

	//zliczanie zgadzajacych sie literek dla loginu do ktorego trzeba wyslac wiadomosc 
	int czyDobraNazwa=0;	
	

	

	for(int i = 0; i<LICZBA_KLIENTOW; i++){
		for(int j = 0; j<strlen(th_data->whoToSend);j++){
			//tu bylo names[i][j]
			if(th_data->ptr_all_users[i].login[j] == th_data->whoToSend[j]){
				printf("Litery są zgodne.\n");	
				czyDobraNazwa = czyDobraNazwa + 1;
			}else{
				printf("Nie zgadzaja sie literki!");
				czyDobraNazwa = 0;
				break;	
			}		
		}
		if(czyDobraNazwa == strlen(th_data->whoToSend)){
			czyDobraNazwa = i;
			printf("Teraz wiem komu wyslac!\n");
			printf("Wysylam do %s\n\n", th_data->ptr_all_users[czyDobraNazwa].login);
			pthread_mutex_lock(&lock);
			while((written_bytes=write(th_data->ptr_all_users[czyDobraNazwa].deskryptor_klienta, th_data->tmpBufor, message_length) < message_length)){
				if(written_bytes == -1){
					printf("Nie udalo sie wyslac wiadomosci do wybranego uzyrtkownika");
					exit(0);
				}
			}
			pthread_mutex_unlock(&lock);
			write(th_data->ptr_all_users[czyDobraNazwa].deskryptor_klienta, "\n", 1);
			break;
		}	
	}

}
//ilosc bajtow i rozlaczenie klienta
	free(t_data);
    pthread_exit(NULL);
}

//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor, struct user *all_users) {
    //wynik funkcji tworzącej wątek	A  
	char message_long[4];
	int result;
	int message_length;

	int written_bytes;
	//liczba odczytanych bajtow zmienna pomocnicza;
	int readed_bytes=0;
	// char read_length[1];
	int read_length;
	//zmienna przechowujaca index tablicy struktur pod ktorym wpisany zostanie wlasnie podlaczony klient
	int just_connected_client;
	
    int create_result = 0;

	struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
	t_data->socket_descriptor = connection_socket_descriptor;

	//wskaznik na wskaznik na tablice struktury w ktorej przechowujemy informacje o userach
	t_data->ptr_all_users = all_users;



	//petla sprawdzajaca czy sa jakies wolne sockety na klienta
	for(int i = 0; i < LICZBA_KLIENTOW; i++){
		if(all_users[i].deskryptor_klienta == 0){
			
			all_users[i].deskryptor_klienta = connection_socket_descriptor;
			//odczytanie dlugosci nazwiska uzytkownika
            // if((readed_bytes=read(all_users[i].deskryptor_klienta, &read_length, sizeof(int))) == -1){
			// 	fprintf(stderr,"Blad przy odczytywaniu dlugosc nazwiska uzytkownika");
            //     // free(t_data);
			// 	// return;
			// }

			 while(readed_bytes < 4){
        		result = read(all_users[i].deskryptor_klienta, message_long + readed_bytes, sizeof(message_long)-readed_bytes);
        		if(result == -1){
            		printf("Blad czytania we ThreadBehavior");
        			}
        			readed_bytes += result;
    			}

    		message_length = (message_long[3] << 24 | message_long[2] << 16 | message_long[1] << 8 | message_long[0]);

    		printf("Dlugosc wiadomosci po przesunieciu bitowym to %d", message_length);

			// while((readed_bytes=read(all_users[i].deskryptor_klienta, &read_length, sizeof(int))) < 4){
			// 	if(readed_bytes == -1){
			// 		fprintf(stderr,"Blad przy odczytywaniu dlugosci loginu uzytkownika");
			// 		// free(t_data);
			// 		// return;
			// 	}
				
			// }

			
			all_users[i].dlugosc_loginu = message_length;
			
            printf("Przeczytano bajtow z loginu %d", readed_bytes);
			//dlugosc loginu to
			printf("Dlugosc loginu to: %d\n", all_users[i].dlugosc_loginu);
			
            while((readed_bytes = read(all_users[i].deskryptor_klienta, all_users[i].login,all_users[i].dlugosc_loginu)) < all_users[i].dlugosc_loginu){
				if(readed_bytes == -1){
					printf("Blad przy odczytywaniu loginu!");
                    // free(t_data);
                    // return;
				}
			}
			
			printf("Przeczytano %d bajtow\n", readed_bytes);
			//petla iterujaca po klientach aby sprawdzic czy ktos nie ma takiego loginu!
			// for(int j=0; j < LICZBA_KLIENTOW; j++){
            //     if(all_users[j].deskryptor_klienta != 0){
            //             if(!strcmp(all_users[i].login,all_users[j].login)){
            //             printf("Jakis uzytkownik ma juz taki login!");
            //             //wyzeruj deskryptor do ktorego przypisales klienta ktory uzyl zajetego loginu;
            //             all_users[i].deskryptor_klienta = 0;
            //             //wyjdz z funkcji handleConnection -> serwer działa dalej
            //             // return;
            //         }
            //     }
				
			// }
			just_connected_client = i;
			break;
        }
    
    }

    //iterujemy po klientach i sprawdzamy czy jakis nie ma takiej samej nazwy
    //przyczyna errorow
    // for(int i=0; i<LICZBA_KLIENTOW; i++){
    //     if((all_users[i].deskryptor_klienta != 0) & (all_users[i].dlugosc_loginu == all_users[just_connected_client].dlugosc_loginu)){
    //         for(int j=0; j<all_users[just_connected_client].dlugosc_loginu;j++){
    //             if(all_users[just_connected_client].login[j] != all_users[i].login[j]){
    //                 printf("Literka w loginie nie jest zgodna!");
    //                 break;
    //             }
    //             if(j == all_users[i].dlugosc_loginu-1){
    //                 printf("Uzytkownicy maja te same loginy");

    //             }
    //         }
    //     }
    // }



   




	printf("New user connected. Say Hi to: %s\n", all_users[just_connected_client].login);

	for(int i=0; i<LICZBA_KLIENTOW;i++){
		printf("Dostepne deskryptory: %d\n", all_users[i].deskryptor_klienta);
	}


	//wyslanie klientowi aktualnie podlaczonych klientow
	//i reszcie nazwy nowo podlaczonego klienta
	for(int i = 0; i<LICZBA_KLIENTOW; i++){
		if((all_users[i].deskryptor_klienta != 0)&&(all_users[i].deskryptor_klienta != all_users[just_connected_client].deskryptor_klienta)){
			while((written_bytes=	write(all_users[i].deskryptor_klienta ,"#\n",2))< 0){
				if(written_bytes == -1){
					fprintf(stderr, "Blad przy wysylaniu do znaku # do jednego z klientow");
					free(t_data);
					return;
				}
			}
			while((written_bytes=write(all_users[i].deskryptor_klienta , all_users[just_connected_client].login, all_users[just_connected_client].dlugosc_loginu))< 0){
				if(written_bytes == -1){
					fprintf(stderr, "Blad przy wysylaniu do znaku # do jednego z klientow");
					free(t_data);
					return;
				}
			}
			while((written_bytes=write(all_users[i].deskryptor_klienta , "\n", 1))< 0){
				if(written_bytes == -1){
					fprintf(stderr, "Blad przy wysylaniu do znaku # do jednego z klientow");
					free(t_data);
					return;
				}
			}
			while((written_bytes=write(all_users[just_connected_client].deskryptor_klienta, all_users[i].login, all_users[i].dlugosc_loginu)) < 0){
				if(written_bytes == -1){
					fprintf(stderr, "Blad przy wysylaniu do znaku # do jednego z klientow");
					free(t_data);
					return;
				}
			}
			while((written_bytes=write(all_users[just_connected_client].deskryptor_klienta, "\n", 1)) < 0){
				if(written_bytes == -1){
					fprintf(stderr, "Blad przy wysylaniu do znaku # do jednego z klientow");
					free(t_data);
					return;
				}
			}
			
			// write(all_users[i].deskryptor_klienta ,"#\n",2);
			//write(all_users[i].deskryptor_klienta , all_users[just_connected_client].login, all_users[just_connected_client].dlugosc_loginu);
			// write(all_users[i].deskryptor_klienta , "\n", 1);
			// write(all_users[just_connected_client].deskryptor_klienta, all_users[i].login, all_users[i].dlugosc_loginu);
			// write(all_users[just_connected_client].deskryptor_klienta, "\n", 1);
		}
		
	}
	
	//wysłanie klientowi znaku mówiącego o tym że skończył odbierać innych klientów
	while((written_bytes=write(all_users[just_connected_client].deskryptor_klienta, "#\n", 2) < 0)){
		if(written_bytes == -1){
			fprintf(stderr, "Blad przy wysylaniu znaku konczenia odbierania klientow");
			free(t_data);
			return;
		}
	}
	// write(all_users[just_connected_client].deskryptor_klienta, "#\n", 2);
	

	
	
	
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
	   free(t_data);
       exit(-1);
	   
    }

    
}







int main(int argc, char* argv[])
{
	printf("Started the server!\n");


	//calloc uzupelnia pola struktury wpisujac zero w typ INT i pusta wartosc w CHAR
	struct user *all_users = malloc(LICZBA_KLIENTOW * sizeof(struct user));

	for(int i=0; i<LICZBA_KLIENTOW;i++){
		all_users[i].deskryptor_klienta = 0;
	}


	int server_socket_descriptor;
	int connection_socket_descriptor;
	int bind_result;
	int listen_result;
	char reuse_addr_val = 1;
	struct sockaddr_in server_address;

	//inicjalizacja gniazda serwera
	
	memset(&server_address, 0, sizeof(struct sockaddr));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);

	server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket_descriptor < 0)
	{
		fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda..\n", argv[0]);
		exit(1);
	}
	
	setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

	bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
	if (bind_result < 0)
	{
		fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
		exit(1);
	}

	listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
	if (listen_result < 0) {
		fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
		exit(1);
	}

	while(1)
	{
		connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
		if (connection_socket_descriptor < 0)
		{
			fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda dla połączenia.\n", argv[0]);
		}
		handleConnection(connection_socket_descriptor, all_users);
		

		
	}

	close(server_socket_descriptor);
	//uwolnienie pamieci
	free(all_users);
	return(0);
    
}



