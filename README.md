# OS_PINTOS_CSE4070
CSE4070, pintos data structure, user program, threads scheduling
<ul>
  <li>data structure usage -> prj_0_2</li>
  <li>user program 1 -> prj_1</li>
  <li>user program 2 -> prj_2</li>
  <li>threads scheduling -> prj_3</li>
</ul>
You may refer to prj_2 for a complete version of prj_1.

<h2>Each Programming Goal</h2>
<h3><b>prj1</b></h3>
<p>userprogram argument passing, user memory access(check validity), system call handler and some system calls</p>
<p>  system call을 구현하고, 유저프로그램을 구현한다. 유저프로그램 구현을 위해서 argument passing이 필요하고, system call을 구현하기 위해 각각의 api 활용을 구현한다. user memory access의 범위를 체크하여 유저 가상메모리 외부 접근 포인터를 차단한다.</p>
<br>
<h3><b>prj2</b></h3>
<p>file system call without concurrency problem(use synchronization to protect a critical section)</p>
<p>  file descriptor table을 구현한다. 기본 파일 시스템에 필요한 시스템 콜을 멀티프로세스 접근을 고려하여 구현한다. create, remove, open, close, filesize, read, write, seek, tell을 구현할 때, 시스템 콜 API를 이해하여 활용한다.</p>
<br>
<h3><b>prj3</b></h3>
<p>alarm clock, thread scheduling</p>
<p>  alarm clock과 priority scheduling을 구현하도록 한다. alarm clock을 구현 시에, 새로운 ready_list 외의 새로운 큐를 만들어 ready와 running을 오가며 계속 기다리는 비효율성을 해결해야 한다. priority scheduling은 구현 시에, 현재 우선순위를 고려하지 않는 round robin을 priority에 따라 schedule하도록 해야 한다.</p>
<br>

<p>If you need more specific information, please refer to document.docx in each project. (except prj1)</p>

<h4>Final project(prj3) passed 100% of assignment requirements (without optional implements)</h4>
