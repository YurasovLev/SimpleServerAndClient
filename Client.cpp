#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>

int main(int argc, char *argv[])
{
    // Проверка аргументов командной строки - Имя клиента, порт сервера, период подключения в секундах.
    if (argc < 4)
    {
        printf("One of the arguments is missing.");
        exit(1);
    }
    std::string userName = argv[1];
    if (userName.length() > 50)
    {
        printf("First argument too long.\n");
        exit(2);
    }
    unsigned short int serverPort = atoi(argv[2]);
    if (serverPort == 0)
    {
        printf("Second argument not a port or equal to zero: %d.\n", serverPort);
        exit(3);
    }
    unsigned int intervalOfConnection = atoi(argv[3]);
    if (intervalOfConnection == 0)
    {
        printf("Third argument not a number or equal to zero: %d.\n", intervalOfConnection);
        exit(4);
    }

    int sock;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // Начинаем цикл, в котором будем с указанным периодом подключатся к серверу
    while (true)
    {
        // Открываем сокет
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            perror("Socket");
            exit(errno);
        }
        // Подключаемся
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("Connect");
            exit(errno);
        }

        // Отправляем размер имени пользователя для синхронизации размера сообщений.
        int userNameLength = htonl(userName.length());
        send(sock, (const char*)&userNameLength, sizeof(int), 0);
        
        char message[89+userName.length()] = "";
        // Получаем текущее время
        timeval curTime;
        gettimeofday(&curTime, NULL);
        int milli = curTime.tv_usec / 1000;
        char buffer [80];
        // Приводим его к нужному формату
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
        // Формируем сообщение, добавляя миллисекунды и имя пользователя
        sprintf(message, "[%s.%03d] \"%s\"", buffer, milli, userName.c_str());

        send(sock, message, 90+userName.length(), 0);

        // Завершаем соединение и закрываем сокет
        close(sock);

        // Вычисляем и ждем определенный период.
        sleep(intervalOfConnection);
    }

    return 0;
}