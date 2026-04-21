## Service Setup and Deployment

**Build:** Set the configuration to **Release** and **x64**, then go to **Build -> Build Solution**.

**Log Preparation:** Manually create the folder `C:\temp\` (or change the path in the code) so the service has permission to write the log file.

**Installation (Admin):** Open **Command Prompt (CMD)** as an Administrator and run:
`sc create "CppHelloWorldService" binPath="path/to/executable.exe"`  
*(Example: "C:\Users\astana\source\repos\Project1\x64\Release\Project1.exe")*

**Run:**
`sc start "CppHelloWorldService"`

---

**Verification:**
After starting the service, check the file `C:\temp\service_log.txt`. You should see the following line:
> **Hello World!**
