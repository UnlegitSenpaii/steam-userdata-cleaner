# steam-userdata-cleaner

Over-engineered user data remover for steam

### Editing the Config File 
The following behavior can be changed using the config file:
##### General Settings

    Backup-Files

>  If this is set to 1, the cleaner will back up the files that will be deleted.
>  
>  Default: 1

    Trace-Logs
> If this is set to 1, the cleaner will print trace logs of what the cleaner is scanning/deleting.
> Useful when the cleaner gets stuck on a file.
> 
> Default: 0

    IgnoreUDataFor-X
> If this is set to an id found in C:\Program Files (x86)\Steam\userdata, the cleaner will ignore the folder while deleting.
> This will keep all of your game saves and other data like steam overlay notes. 
> Multiple IDs can be defined using the integer after the - as an incrementor:
> ```
>  IgnoreUDataFor-0=388777777
>  IgnoreUDataFor-1=388888888
>  IgnoreUDataFor-2=388999999
> ```
> Default: 0

### Compiling the Project

Because the cleaner is a subproject of "redcore.cf", you have different compile options, and I'm too lazy to remove them. You can choose any compile setting, the one in the release tab is compiled with Release - NVIDIA Win32 - Visual Studio 2022.
