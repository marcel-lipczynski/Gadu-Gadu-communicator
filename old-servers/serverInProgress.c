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
	
	//zmienna do konwersji na int
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

	
	//odczytanie długości wiadomości która zostanie wysłana
	while((readed_bytes=read(th_data->socket_descriptor, &message_length, sizeof(int)) < 1)){
		if(readed_bytes == -1){
			printf("Blad podczas odczytywania dlugosci wiadomosci");
		}
	}

	
	printf("Dlugosc nadchodzacej wiadomosci to %d\n", message_length);

	// read(th_data->socket_descriptor, th_data->bufor, 100);
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
	
	// To zwraca warningi przy kompilacji!!!!
	// printf("Dlugosc lancucha whoTosend  %d\n",strlen(th_data->whoToSend));
	// printf("Dlugos names[0] %d\n",strlen(names[0]));
	
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

		


	//for(int i = 0; i<3; i++){
	//	if(names[i] == th_data->whoToSend){
	//		printf("Znaleziono takiego uzytkownika :)");
	//		if(clients[i] != 0)
	//			write(clients[i], th_data->bufor, 100);
		//}
			
		//if((clients[i] != 0)&&(clients[i] != th_data->socket_descriptor))
		//	write(clients[i], th_data->bufor, 100);
//}
}
//ilosc bajtow i rozlaczenie klienta
	free(t_data);
    pthread_exit(NULL);
}

//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor, struct user *all_users) {
    //wynik funkcji tworzącej wątek	A  

	//liczba odczytanych bajtow zmienna pomocnicza;
	int readed_bytes;
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
			if((readed_bytes=read(all_users[i].deskryptor_klienta, &read_length, sizeof(int))) == -1){
				printf("Blad przy odczytywaniu dlugosc nazwiska uzytkownika");
				exit(0);
			}

			//konwertowanie odebranej dlugosc z char na int
			all_users[i].dlugosc_loginu = read_length;
			// read_length[0] - '0';

			//dlugosc przekonwertowanego loginu to
			printf("Dlugosc loginu to: %d\n", all_users[i].dlugosc_loginu);
			//TODO -> czytanie w pętli i sprawdzanie czy nie ma bledu przy czytaniu
			while((readed_bytes = read(all_users[i].deskryptor_klienta, all_users[i].login,all_users[i].dlugosc_loginu)) < all_users[i].dlugosc_loginu){
				if(readed_bytes == -1){
					printf("Blad przy odczytywaniu loginu!");
				}
			}
			printf("Przeczytano %d bajtow\n", readed_bytes);
			just_connected_client = i;
			break;
		}else{
			//TODO jesli nie ma wolnego miejsca nalezy go nie podlaczac!
			printf("TODO");
		}
	}
		// read(all_users[ile].deskryptor_klienta, all_users[ile].login, 100);
		// read(t_data->ptr_all_users[ile]->deskryptor_klienta,t_data->ptr_all_users[ile]->login, 100);
		// printf("check if all_users shows the same what ptr_all_users show: %d\n", all_users[just_connected_client].deskryptor_klienta);
	
	printf("New user connected. Say Hi to: %s\n", all_users[just_connected_client].login);

	for(int i=0; i<LICZBA_KLIENTOW;i++){
		printf("Dostepne deskryptory: %d\n", all_users[i].deskryptor_klienta);
	}

	



	//wyslanie klientowi aktualnie podlaczonych klientow
	//i reszcie nazwy nowo podlaczonego klienta
	
	for(int i = 0; i<LICZBA_KLIENTOW; i++){
		if((all_users[i].deskryptor_klienta != 0)&&(all_users[i].deskryptor_klienta != all_users[just_connected_client].deskryptor_klienta)){
			write(all_users[i].deskryptor_klienta ,"#\n",2);
			write(all_users[i].deskryptor_klienta , all_users[just_connected_client].login, all_users[just_connected_client].dlugosc_loginu);
			write(all_users[i].deskryptor_klienta , "\n", 1);
			write(all_users[just_connected_client].deskryptor_klienta, all_users[i].login, all_users[i].dlugosc_loginu);
			write(all_users[just_connected_client].deskryptor_klienta, "\n", 1);
		}
		
	}
	
	write(all_users[just_connected_client].deskryptor_klienta, "#\n", 2);
	

	//wysyłanie do wszystkich klientow nazwy nowo podlaczonego klienta
	
	// ile++;
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }

    //TODO (przy zadaniu 1) odbieranie -> wyświetlanie albo klawiatura -> wysyłanie
}

int main(int argc, char* argv[])
{
	printf("Started the server!\n");


	//calloc uzupelnia pola struktury wpisujac zero w typ INT i pusta wartosc w CHAR
	struct user *all_users = malloc(LICZBA_KLIENTOW * sizeof(struct user));

	for(int i=0; i<LICZBA_KLIENTOW;i++){
		all_users[i].deskryptor_klienta = 0;
	}

	

	
	
	// for(int i = 0; i< LICZBA_KLIENTOW; i++)
	// {	
	// 	clients[i]=0;
	// 	memset(names[ile], 0, sizeof(names[ile]));
	// }
	// ile=0;
	

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
			exit(1);
		}

		handleConnection(connection_socket_descriptor, all_users);
	}

	close(server_socket_descriptor);
	return(0);
}



