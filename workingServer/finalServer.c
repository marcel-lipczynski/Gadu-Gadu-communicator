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




#define SERVER_PORT 1234 // port servera na ktorym nasluchujemy uzytkownikow
#define LICZBA_KLIENTOW 10 // maksymalna ilość klientów - póki co 10 dla testow
#define LOGIN_LENGTH 100 // długość loginu użytkownika -> max 100 znaków
#define MAX_MESSAGE_LENGTH 10000; // dlugosc wiadomosci do 10000 znakow mozna dac wiecej czego dusza pragnie

pthread_mutex_t lock; // przy odbieraniu wiadomosci od kilku na raz

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
	char bufor[10000];
	char tmpBufor[10000];
	//do kogo wyslac wiadomosc?
	char *whoToSend;
	// char **pointer_loginy; // wskaznik na tablice loginow
	// int *pointer_klienci; // wskaznik na tablice klientow
	struct user *ptr_all_users;

};

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data)
{
    //bufor sluzacy do czytania integera bajt po bajcie a potem zamiana go na int
    char message_long[4];
	//zmienna do zliczania czytanych bajtow 
    int result;
	//sekwencja mowiaca o wylogowaniu sie klienta
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





	//czytanie 4 bajtow czyli calego integera
    while(readed_bytes < 4){
        result = read(th_data->socket_descriptor, message_long + readed_bytes, sizeof(message_long) - readed_bytes);
        if(result == -1){
            printf("Blad czytania we ThreadBehavior");
        }
        readed_bytes += result;
    }

	//przesuniecie bitowe w celu uzyskania inta
    message_length = (message_long[3] << 24 | message_long[2] << 16 | message_long[1] << 8 | message_long[0]);

    printf("Dlugosc wiadomosci po przesunieciu bitowym to %d", message_length);
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
        printf("\n\n USER IS DISCONNECTING!\n");

		//iteruje po wszystkich elementach tablicy struktur
        for(int i=0; i<LICZBA_KLIENTOW; i++){
			//jesli jest tam aktywny deskryptor czyli wiekszy od zera
            if(th_data->ptr_all_users[i].deskryptor_klienta > 0){
				//to jesli jest to uzytkownik inny od tego co sie odlacza
				if(th_data->ptr_all_users[i].deskryptor_klienta != th_data->socket_descriptor){
					printf("Sending message to %s\n", th_data->ptr_all_users[i].login);
					//to wyslij im informacje o tym ze ktos sie zaraz rozlaczy
					if(write(th_data->ptr_all_users[i].deskryptor_klienta,"#!#\n",4) == -1){
						printf("Error przy wysylaniu informacji o wylogowaniu uzytkownika");
					}
					//wyslij informacje jaki uzytkownik sie rozlaczy
					if(write(th_data->ptr_all_users[i].deskryptor_klienta, th_data->tmpBufor,message_length)==-1){
						printf("Error przy wysylaniu wiadomosci ktora wyslal uzytkownik");
					}else{
						write(th_data->ptr_all_users[i].deskryptor_klienta, "\n",1);
					}
					
				}
				else{

					//W przeciwnym przypadku 
					//jesli to odlaczajacy sie user to wyzeruj jego tablice elementow i login
					if(th_data->ptr_all_users[i].deskryptor_klienta == th_data->socket_descriptor){
						th_data->ptr_all_users[i].deskryptor_klienta = 0;
						memset(th_data->ptr_all_users[i].login,0,LOGIN_LENGTH);
					}
				}
            }
        }
        
		printf("User disconnected succesfully, BYE!\n");
		//zwolnij strukture na ktorej operowal watek
        free(th_data);
		//zabij watek
        pthread_exit(NULL);
		//wyjdz z funkcji
		
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
			//mutexy niezbedne do wspolbieznego przesylania wiadomosci do jednego uzytkownika przez kilku uzytkownikow!
			pthread_mutex_lock(&lock);
			while((written_bytes=write(th_data->ptr_all_users[czyDobraNazwa].deskryptor_klienta, th_data->tmpBufor, message_length) < message_length)){
				if(written_bytes == -1){
					printf("Nie udalo sie wyslac wiadomosci do wybranego uzyrtkownika");
					exit(0);
				}
			}
			//oddanie mutexu - odblokowanie go
			pthread_mutex_unlock(&lock);
			write(th_data->ptr_all_users[czyDobraNazwa].deskryptor_klienta, "\n", 1);
			break;
		}	
	}

}
	//zwolnienie pamieci i zabicie watku w momencie wyjscia z petli while(true)
	free(t_data);
    pthread_exit(NULL);
}

//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor, struct user *all_users) {

	//odczytanie integera jako 4 bajtow ;
	char message_long[4];
	//zmienna sprawdzajaca stan odczytanych bajtow ;
	int result;
	//zmienna w ktorej zapisujemy odczytanego integera;
	int message_length;

	int written_bytes;
	//liczba odczytanych bajtow zmienna pomocnicza;
	int readed_bytes=0;
	
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
			
			 while(readed_bytes < 4){
        		result = read(all_users[i].deskryptor_klienta, message_long + readed_bytes, sizeof(message_long)-readed_bytes);
        		if(result == -1){
            		printf("Blad czytania we ThreadBehavior");
        			}
        			readed_bytes += result;
    			}

    		message_length = (message_long[3] << 24 | message_long[2] << 16 | message_long[1] << 8 | message_long[0]);

    		printf("Dlugosc wiadomosci po przesunieciu bitowym to %d", message_length);

			

			//przypisanie dlugosci loginu do uzytkownika
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

    // iterujemy po klientach i sprawdzamy czy jakis nie ma takiej samej nazwy
    for(int i=0; i<LICZBA_KLIENTOW; i++){
        if((all_users[i].deskryptor_klienta != 0) & (all_users[i].dlugosc_loginu == all_users[just_connected_client].dlugosc_loginu) && (all_users[i].deskryptor_klienta != all_users[just_connected_client].deskryptor_klienta)){
            for(int j=0; j<all_users[just_connected_client].dlugosc_loginu;j++){
                if(all_users[just_connected_client].login[j] != all_users[i].login[j]){
                    printf("Literka w loginie nie jest zgodna!");
                    break;
                }
                if(j == all_users[i].dlugosc_loginu-1){
                    printf("Uzytkownicy maja te same loginy");
					//wyslij pusty znak mowiacy o tym ze maja te same loginy!
					if(write(all_users[just_connected_client].deskryptor_klienta, "\n",1)==-1){
						printf("Error przy informowaniu o zajetym loginie");
					}
					//wyzeruj deskryptor z tablicy struktur
					all_users[just_connected_client].deskryptor_klienta=0;
					//wyzeruj login uzytkownika bo juz go z nami nie ma :(
					// all_users[just_connected_client].login="";
					memset(all_users[just_connected_client].login,0,LOGIN_LENGTH);
					//zwolnij pamiec w ktorej zaalokowano t_data
					free(t_data);
					//zakoncz dzialanie funkcji handleConenction;
					return;
                }
            }
        }
    }



   




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

	listen_result = listen(server_socket_descriptor, LICZBA_KLIENTOW);
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



