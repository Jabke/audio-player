## Настройка зависимостей 
Данная библиотека использует api linux - pulse-audio,
то есть для работы с ней нужно установить библиотеку для
работы с этим апи:
```
sudo apt install pulseaudio
``` 
```
sudo apt install libpulse-dev
```

ТЗ (что должно в итоге получиться):
1) Возможный формат проигрывания - mp3 или wav
2) Возможность в будущем расширить количество поддерживаемых форматов
3) Объект player:
    1) Ему передаётся путь до трека, он проигрывается, в случае если формат не поддерживается
или искомый файл не найден - либо выставляется соответствующий флаг, либо выкидывается исключение
    2) Есть возможность поставить трек на паузу или остановить вовсе
    3) Есть возможность последовательного воспроизведения музыки из определённой папки, а
так же закольцовывания её, если это нужно;
    4) В идеале это должна быть не асинхронная библиотека;
    5) Потокобезопасность - библиотека должна быть потокобезопасной;
4) Требования для первого релиза - объект player, возможность воспроизводить и переключать 
треки хотя бы одного формата, возможность автопереключения треков в папке, возможность регулировать
уровень громкости.
