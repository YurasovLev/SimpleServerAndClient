# Вспомогательный скрипт, запускающий введеное число клиентов. Каждый из клиентов подключается через количество секунд, равно его номеру.
# Сигнатура — ./RunClient.sh PORT NUMBER_OF_CLIENTS
for ((i=1; i <= $2; i++))
do
    ./Client User$i $1 $i &
done