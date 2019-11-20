Process Diagnostician (프로세스 진단사)
=======================================
<br/>

<div style="text-align:right">2019061658 김태현</div>
<br/>

### 1. 개발 돌입하기 전에..
--------------------------
#### Q1. 이 프로그램(Process Diagnostician)을 만들기로 마음 먹은 이유는 무엇인가? 
##### A. <2019-11-11 23:15>
>컴퓨터는 우리가 생각하는 것보다 훨씬 **위험에 노출**되어 있다.
<br/>
>옛날, 필자는 좀비PC에 걸려 한껏 고생했던 기억이 있다. 그 당시에 어떤 프로세스가 바이러스인지 알았다면 포맷이라는 마지막 수단까지 쓰지 않았을 것이다. 그 이후로, 출처가 불분명한 프로세스를 보면 항상 구글링을 하거나 바이러스 검사를 해본다. 그래서 그것을 도울 수 있는 **프로세스들을 진단하는 프로그램**을 만들기로 했다.<br/>


#### Q2. 프로그램을 어떤 식으로 구현할 예정인가?
##### A. <2019-11-11 23:30>
- C언어로 구현할 예정이다.<br/>
- 프로세스들의 정보를 얻기 위해, 우분투의 터미널 명령어인
 >pstree -p

 를 통해 얻을 수 있는 pid들을 하나씩 알아낼 것이다.
 <br/>
 그리고나선,

 >file /proc/***pid***/exe

 pid를 이용하여 프로그램의 경로를 알아낸 뒤, VirusTotal에서 제공해주는 API를 사용해 프로그램의 위험성을 진단할 것이다.
- 테스트는 Ubuntu 18.04에서 이루어질 예정이다.


### 2. ** 개발 환경 및 요구 조건
---------------------------
#### 개발 환경 : ubuntu-18.04.3-desktop-amd64 설치 패키지 권장 옵션으로 설치.
#### 빌드 시 필요한 패키지 목록 : 추후 공지.
#### 프로그램 동작에 필요한 패키지 목록 : net-tools, build-essential
##### * net-tools 설치 방법 : Terminal 에서 "sudo apt install net-tools"를 입력한 후 다운로드 받는다.
##### * build-essential 설치 방법 : 위의 방법과 동일하나, net-tools을 build-essential로 바꿔 하면 된다.


### 3. ** 프로그램 실행 방법 (Clone 방법)
--------------------------------------------
#### 추후 안내.


### 4. ** 프로그램 실행 화면
------------------------
#### 추후 업로드.


### 5. ** 기능 안내
------------------
#### 추후 안내.


### 6. ** TroubleShooting Note
------------------------------
>\<YYYY-MM-DD HH:MM\> Error, Solution. \<\< **이런 식으로 작성될 것입니다.**

### 7. ** 기능 구현 일지
--------------------------
**\<2019-11-13 22:16\>** Terminal에서 나오는 결과 값을 구하고 싶다. 리눅스API를 사용할까? 아니면 터미널의 출력 값을 파일로 받아올까? ( '> .txt' 이용)
<br/>
<br/>
**\<2019-11-18 15:39\>** 프로그램 상에서 터미널 명령어를 입력하고 싶어서 방법을 알아보았다. system("command"); 함수를 쓰면 된다고 한다. stdlib.h 를 포함시킨 후 사용 가능했다. 프로그램 실행 Path에 "pstree -p" 결과값을 가진 텍스트 파일("pstree.txt")을 저장하게 했다. 

