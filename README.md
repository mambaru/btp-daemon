# Общая информация

BTP - система для анализа производительности веб-приложений.
Идея такая: веб-приложение замеряет время выполнения каких-то интересных участков кода (например, запросы к базе/кэшу/внешним сервисам) и сообщает это время BTP. BTP обрабатывает эти данные и выводит статистику в интерфейсе. При этом для измерения времени используется обычный код на вашем языке программирования, без обязательного использования каких-либо native extensions, поэтому, хотя мы используем BTP из PHP, но тем не менее BTP можно использовать с любоым языком программирования.

Ссылки на все части проекта:

 - http://github.com/mambaru/btp-daemon/ - демон
 - http://github.com/mambaru/btp-webui/ - веб-интерфейс
 - http://github.com/mambaru/btp-api/ - апи

![обращения ко всем серверам мемкэша](http://i.imgur.com/kiY0H.png)

Посмотреть больше картинок: http://imgur.com/a/idFF6


# Установка и настройка

## Требования

 - [Kyoto Cabinet][1]
 - [Intel Threading Building Blocks][2] 
 - GCC 4.5+
 - Boost
 - CMake

В Gentoo нужно установить dev-db/kyotocabinet, dev-cpp/tbb, dev-libs/boost, dev-util/cmake
В Debian: libtbb2, libtbb-dev, libboost-dev, libboost-thread-dev libboost-program-options-dev, cmake и уже из исходников kyotocabinet
  
    http://fallabs.com/kyotocabinet/pkg/
    http://threadingbuildingblocks.org/file.php?fid=77

## Установка

    git clone git://github.com/mambaru/btp-daemon.git
    cd btp-daemon
    make all
    ./release/daemon --help

## Установка веб-интерфейса
	
	git clone git://github.com/mambaru/btp-webui.git
	
Далее эту директорию нужно положить так, чтобы она была видна веб-сервером. Если демон и веб-интерфейс находятся на одном сервере, ничего дальше делать не нужно.
Если веб-интерфейс и демон находятся на разных серверах, нужно отредактировать config.js и прописать адрес демона.

# Использование

BTP состоит из двух частей - это демон и веб-интерфейс. Они независимы друг от друга, и при необходимости можно использовать несколько демонов (dev / production) с одним веб-интерфейсом.
В качестве протокола обмена используется JSON-RPC. Счётчики можно передавать как по TCP, так и по UDP, запросы на получение данных лучше делать по TCP.

Например:

    user@host $ telnet 127.0.0.1 22400
    {"jsonrpc":"2.0","method":"put","params":{"script":"test.php","items":{"mysql":{"db7":{"connect":[2964],"select":[3595,2009,6777]}}}}}
    ^]
    telnet> Connection closed.

Такой запрос сообщает BTP, что был запущен скрипт test.php, который обратился к сервису mysql, а конкретно к серверу db7, причём операция connect была вызвана 1 раз и заняла почти 3мс, а select - 3 раза, 3.5, 2 и 6.8 мс.

После этого в интерфейсе можно увидеть следующее:

 - /#service/ - в списке появился mysql
 - /#script/ - в списке появился test.php
 - /#service/mysql/ - в списке "Серверы, которые обслуживают сервис mysql" появился сервер db7 и ниже видны операции select и connect. Здесь показывается вся учитываемая нагрузка, которая идёт на этот сервер
 - /#script/test.php/ - в списке "Сервисы, которые используются из test.php" появился mysql. Если кликнуть на mysql, попадём в ..
 - /#script/test.php/mysql/ - какие операции сервиса mysql и в каком количестве используются из скрипта test.php

Теперь сделаем второй запрос:

	user@host $ telnet 127.0.0.1 22400
	{"jsonrpc":"2.0","method":"put","params":{"script":"test2.php","items":{"mysql":{"db7":{"connect":[2964],"select":[3595,2009,6777]}}}}}
	^]
	telnet> Connection closed.

 - /#service/ - без изменений
 - /#script/ - в списке появился test2.php
 - /#service/mysql/ - появилась второй набор точек, чуть правее по оси времени. в остальном без изменений
 - /#script/test.php/ - без изменений
 - /#script/test.php/mysql/ - без изменений (второй точки не появилось, потому что на этой странице показываются запросы только из test.php)
 - /#script/test2.php/mysql/ - 1 набор точек, соответствующий новому набору из /#service/mysql/

Через какое-то время (до 30 минут) на странице /#service/mysql/ появится список скриптов, которые наиболее активно используют сервис mysql.


# API

## Отправка счётчиков

	git clone git://github.com/mambaru/btp-webui.git

Паттерн использования примерно следующий. Например, у вас есть код:

```php

function get_something_from_database($parameters) {
	$data = Database::getConnection()->query('select * from table where some_id=12');
	$result = array();
	foreach ($data as $row) {
		$result[] = ....
	}
	return $result;
}

```


```php

// вариант 1 - явное удаление счётчика
function get_something_from_database($parameters) {
	$counter = new Stat_Btp_Counter(Stat_Btp_Request::getLast(), array(
		'srv' => 'db7',
		'service' => 'mysql',
		'op' => 'select',
	));
	$data = Database::getConnection()->query('select * from table where some_id=12');
	$counter->stop();

	$result = array();
	foreach ($data as $row) {
		$result[] = ....;
	}
	
	return $result;
}

// вариант 2 - с помощью деструктора
function get_something_from_database($parameters) {
	$counter = new Stat_Btp_Counter(Stat_Btp_Request::getLast(), array(
		'srv' => 'db7',
		'service' => 'mysql',
		'op' => 'select',
	));

	$data = Database::getConnection()->query('select * from table where some_id=12');
	$result = array();
	foreach ($data as $row) {
		$result[] = ....;
	}
	return $result;
}

```

При этом в варианте 1 будет измерено только время работы Database::getConnection()->query..., в варианте 2 меряется полное время начиная со второй строки функции о до выхода из неё.
Следует учесть, что в текущей реализации счётчики накапливаются и отправляются небольшими группами. Для web-скриптов и некоторых cron-скриптов такое поведение желательно, но если например ваш cron-скрипт большую часть времени проводит в sleep, то лучше отправлять счётчики принудительно перед засыпанием: Stat_Btp_Request::getLast()->close()

## Получение данных из демона

Это потребуется, в частности, если вы захотите сделать свою реализацию графиков. Для тестовых вызовов можно использовать cmd.php, например 
```
Запись вызова в той нотации, которая будет использована ниже:
    get_list({service:"?"})
Запись вызова в командной строке:
    php cmd.php get_list host=ip_of_btp_daemon service=?
Запись вызова в PHP-коде:
    $conn = new JsonRpc_Connection(array('host'=>'127.0.0.1','port'=>22400));
    print_r($conn->request('get_list',array('service'=>'?'))->get());
```

### Список операций

```delete({service:"value", op:"value"}) => bool```
удаление данных операции

```delete({service:"value", server:"value"}) => bool```
удаление данных сервера

```delete({service:"value"}) => bool```
удаление данных сервиса

```delete({script:"value"}) => bool```
удаление данных скрипта

```get_list_advanced({service: '?', limit: 10, sortby: 'count'})```
список сервисов, отсортированный по count (возможные сортировки: count - число запросов, total - общее время которое равно count*avg, perc80 - значение перцентиля)

```get_list_advanced({service: 'value', op: 'value', script: '?', limit: 10, sortby: 'count'})```
список скриптов, которые используют операцию сервера, отсортированный по count (возможные сортировки: count - число запросов, total - общее время которое равно count*avg, perc80 - значение перцентиля)

```get_list_advanced({service: 'value', op: 'value', server: '?', limit: 10, sortby: 'count'})```
список серверов, которые предоставляют операцию сервера, отсортированный по count (возможные сортировки: count - число запросов, total - общее время которое равно count*avg, perc80 - значение перцентиля)

```get_list({server: .., server: .., op: .., script: ..})```
один из четырёх ключей должен быть "?", остальные могут быть опущены/пустыми строками, либо указывать значение. При этом не все сочетания имеют смысл, например {op: "?", service: "val"} - список операций на сервисе val, {op: "?", server: "val"} - не имеет смысла (поскольку op привязывается к сервису, а не к серверу), поэтому server проигнорируется

```put({script:'index.php',items:{service_name: {server_name: {op_name: [ts1, ts2, ..tsn] } } } )```
добавление статистики. передаются имя скрипта, и счётчики по сервисам/серверам/операциям

# Производительность

В "Мамбе" BTP обрабатывает примерно 2..3 млн счётчиков в секунду. Строчка из top:
<code>3185 user      20   0 17.8g  14g 2.1g R  186 61.8  14072:14 btp_release    </code>

Память идёт на буфера, кэш, и на хранение первичных данных до их обсчёта. При меньшей нагрузке использование памяти будет меньшим.


# Лицензия

Программа-демон разработана в [компании "Мамба"][4] и распространяется по лицензии GNU GPL ver. 2.  Web-интерфейс разработан в [компании "Мамба"][4] и распространяется по лицензии MIT.
Разработчик - [Илья Шаповалов][5]. В демоне используются следующие библиотеки:

 - faslib - GPL2. (c) [Владимир Мигашко, компания "Мамба"][7]
 - kyoto cabinet - GPL3 + Foss License Exception. (c) [FAL Labs][6]
 - intel tbb - GPL2. (c) Intel
 - boost: Boost License. 

Библиотеки, используемые в web-интерфейсе, см. в подпапках веб-интерфейса.


  [1]: http://fallabs.com/kyotocabinet/
  [2]: http://threadingbuildingblocks.org/ver.php?fid=182
  [3]: git://github.com/mambaru/btp-webui.git
  [4]: http://corp.mamba.ru/
  [5]: http://github.com/shepik/
  [6]: http://fallabs.com/
  [7]: http://code.google.com/p/faslib/

