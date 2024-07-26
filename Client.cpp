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
    // Проверяем, все ли аргументы были введены.
    if (argc < 4) // Всего аргументов должно быть четыре.
    {
        // Если аргументов не достаточно, выводим поясняющее сообщение.
        printf("Not all arguments are entered");
        printf("Client NAME PORT INTERVAL");
        printf("NAME — The user name, from 1 to 50 characters long");
        printf("PORT — The port for connection to the server");
        printf("INTERVAL — The number of seconds between server connections");
        exit(1);
    }


    // Считываем и преобразуем все аргументы.
    std::string userName = argv[1];
    unsigned short int serverPort = atoi(argv[2]);
    unsigned int intervalOfConnection = atoi(argv[3]);


    // Проверяем все аргументы
    if (userName.length() > 50) // Ограничение длины имени пользователя.
    {
        printf("NAME too long.\n");
        exit(2);
    }
    if (serverPort == 0) // Проверка того, что порт задан цифрами и его не задали равным нулю (Технический порт системы).
    {
        printf("PORT not a number or equal to zero: %d.\n", serverPort);
        exit(3);
    }
    if (intervalOfConnection == 0) // Проверка того, что интервал задан числами и не равен нулю.
    {
        printf("INTERVAL not a number or equal to zero: %d.\n", intervalOfConnection);
        exit(4);
    }


    // Вычисляем размер имени пользователя при передаче по сетевому протоколу. Необходимо для синхронизации.
    int userNameSize = htonl(userName.length());

    // Создаем переменную для сокета и адреса подключения к серверу.
    int sock;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);


    // Цикл подключения к серверу с заданным интервалом
    while (true)
    {
        // Открываем сокет
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) // Обрываем программу если не удалось открыть сокет.
        {
            perror("Socket");
            exit(errno);
        }
        // Подключаемся
        int connectResult = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (connectResult < 0) // Обрываем программу если не удалось подключится.
        {
            perror("Connect");
            exit(errno);
        }


        // Отправляем размер имени пользователя для синхронизации размера сообщений.
        send(sock, (const char*)&userNameSize, sizeof(int), 0);
        

        // Создаем массив для сообщения соответствующей длины.
        // 89 — это размер сообщения без имени пользователя.
        char message[89+userNameSize] = "";


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

        // Завершаем соединение.
        close(sock);

        // Приостанавливаем программу на заданное интервалом количество секунд.
        sleep(intervalOfConnection);
    }

    return 0;
}