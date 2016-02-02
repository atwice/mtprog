mtprog
======

Tasks for stepic.org course "Multithreading Programming

## bashlike
Задача на безымянные каналы.

Пусть есть длинная команда вида:
'''
who | sort | uniq -c | sort -nk1
'''

Надо её прочитать из STDIN, выполнить и STDOUT записать в файл */home/box/result.out*.

Конечно, я пишу не в /home/box/, а в ./

## namedpipes

Задача на именованные каналы.

Напишите программу, которая создает два именованных канала - /home/box/in.fifo и /home/box/out.fifo

Пусть программа читает in.fifo и все прочитанное записывает в out.fifo.

