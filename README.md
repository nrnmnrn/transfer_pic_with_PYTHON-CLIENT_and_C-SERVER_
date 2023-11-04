# transfer_pic_with_PYTHON-CLIENT_and_C-SERVER
### 概述
方法分成 origin(server、client) 跟 improved(server_improved、client_improved) 兩種。  
所有的測試皆是通過虛擬機和本機模擬兩個本地透過socket傳遞圖片測試出的結果。  

* **環境**  
虛擬機端作業系統 / 程式語言: Ubuntu 20.04 / C Language  
本地端作業系統 / 程式語言: Windows11 / Python3.8

* **運行方式**  
server先開(虛擬機)，再開client(本地)。記得虛擬機要連上局網。server端也請在程式目錄下再創一個 image資料夾，因為所有傳送過來的圖片皆會送到這，否則會出錯。


### origin:
有機率出現少傳資料的問題(測下來每次少傳都是8個bytes，也不知為何)。所以增加time.sleep(1)的方式來減少發生機率。sleep的時間越少，資料不全的機率越大，也就是圖片會顯示不出來。

### improved:  
參考了[這篇](https://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data)修改原本的傳送資料方式。即使不需要time.sleep()也能使所有圖片完整送達。

### 方法差異
origin把圖片大小和圖片資料分開傳送。先sendall(image_size)，再送sendall(image)  
improved把圖片大小和圖片一起傳送。sendall(image_size + image)
