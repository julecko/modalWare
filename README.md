# Introduction
This project is a proof of concept, if it is really possible to create malware based on components.
# Initial tought
It always starts with an idea. An idea, to create something unique. An idea, to do something differently.

During my time learning website development, how all of thoose structures work, how can i split my html into components, i got an idea, if it would be possible to create malware, which is purely based on components. If malware infects computer, extending its functionality is often hard. Most malwares are compiled/created to do single purpose and thats it. Of course, there are frameworks like metasploit ruby framework for creating malware, but they allow you to create malware at place, and later extending it, again, too complicated. But what if, you could create piece of code, which you can either embed staticly into program at compilation time or you can just send it later and still achieve same result. Thats what this project demonstrates. An option, to have malware, which is expanded at any time.

# Compilation
Compile using Visual Studio, small separate modules can be compiled using `compile_library.bat`

# Next
- [x] Create configuration standard for main program where will be startup functions and loading of this configuration
- [ ] Finish command and controll where it will return call of function if it matches the standard
- [ ] Add command to call function from dll
- [ ] Add clock to call functions automaticly
- [x] Add configuration file creation for my discord communication module