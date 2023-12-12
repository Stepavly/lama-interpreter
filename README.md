# lama-interpreter

Итеративный интерпретатор для языка Lama.

## Исходный код

Исходный код интерпретатора находится в `byterun/byterun.c`. 

## Сборка

Чтобы собрать интерпретатор в корне проекта необходимо выполнить `make all`. Собранный интерпретатор будет находится в `byterun/byterun`.

## Тестирование

Чтобы протестировать интерпретатор необходимо в корне проекта выполнить `make all && chmod u+x test.sh && ./test.sh` (команда `make all` не обязательна, если интерпретатор был собран).

Чтобы сравнить время работы реализованного интерпретатора с рекурсивным интерпретатором необходимо в корне проекта выполнить `make all  && chmod u+x performance.sh && ./performance.sh <путь до lamac>`, где `<путь до lamac>` может быть `lamac`, если `lamac` доступен глобально (команда `make all` не обязательна, если интерпретатор был собран).

## Сравнение производительности

Результат запуска `performance.sh`:

```
Executing lamac interpreter

real    0m3,714s
user    0m3,694s
sys     0m0,020s

Executing iterative interpreter

real    0m1,635s
user    0m1,618s
sys     0m0,016s
```