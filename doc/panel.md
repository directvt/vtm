# The panel
In release [v0.9.9v](https://github.com/netxs-group/vtm/releases/tag/v0.9.9v) a slot for panels was introduced. 


## settings
The following lines where added to the [example `settings.xml`](https://github.com/netxs-group/vtm/blob/master/doc/settings.md#typical-config--configvtmsettingsxml):
```xml
    <panel> <!-- Desktop info panel. -->
        <cmd = ""/> <!-- Command-line to activate. -->
        <cwd = ""/> <!-- Working directory. -->
        <height = 1 /> <!-- Desktop space reserved on top. -->
    </panel>
```
Here you can set what command provides the lines to display in the panel.

## making a script for the panel
For this script I will be using `python3`, but I guess you can adapt this to your favorite language. A bash version will be below, to help if you don't understand python.

First set the path to your script in `settings.xml` bt changing the `config/panel/cmd` tag to contain the path.
Then make your script at that location, and we can start.

First, the concept.
The panel works by running your command in a small terminal at the top. This means you will have a scrollbar. To prevent that, (and thus to save a little amount of memory,) print the following escape code to the screen: `\e[[?1049h`.

This means our script now looks like this:
```python
#!/usr/bin/env python3
# my custom panel

print("\033[?1049h") # remove scrollback

```

If you print data, that data will push the previous data off-screen. The script has to run in a loop to make sure the data gets updated. So, add a loop which collects data and prints that.
```python
#!/usr/bin/env python3
# my custom panel
from time import sleep # import sleep() function

print("\033[?1049h") # remove scrollback
while True: # loop infinitly
    data='My data' # collect data
    print(data) # print data
    sleep(1) # wait 1 second before looping back to the start

```
This will print `My data` to the panel every 1 second.

I guess we now want to print some actual data. Lets print the time.
    In python, we get that from `datetime.datetime.now()`. `datetime.datetime.now()` returns the time in this format: `YYYY-MM-DD HH:MM:SS.SSSSSS`. We don't want the date or nanoseconds, so lets split those off: `datetime.datetime.now().split(' ')[1].split('.')[0]` returns `HH:MM:SS`. perfect.
Now lets put that in the script:
```python
#!/usr/bin/env python3
# my custom panel
from time import sleep # import sleep() function
import datetime

print("\033[?1049h") # remove scrollback
while True: # loop infinitly
    time=datetime.datetime.now().split(' ')[1].split('.')[0] # get current time
    print(time) # print data: the time
    sleep(1) # wait 1 second before looping back to the start

```
This will print the current time to the panel, and you can see it update every second.

You probably want to add more data, and that is possible. Just collect more data, and add the variables to the `print()` function.
Here I added a date and the text `Hello world!`, and I seperated them all by ` | `:
```python
#!/usr/bin/env python3
# my custom panel
from time import sleep # import sleep() function
import datetime

print("\033[?1049h") # remove scrollback
while True: # loop infinitly
    time=datetime.datetime.now().split(' ')[1].split('.')[0] # get current time
    date=datetime.datetime.now().split(' ')[0]
    print(time,'|',date,'|','Hello world!') # print data: time | date | Hello world!
    sleep(1) # wait 1 second before looping back to the start

```
A full script to base your own panel on can be found [here](https://github.com/Vosjedev/vtm-panel/).

A bash adaptation of the script we just created in python (so people that don't 'speak' python know what's going on):
```shell
#!/usr/bin/env bash
# my custom panel

echo -e "\e[?1049h" # remove scrollback

while true; do
    time="$(date +%X)" # get time (in local format)
    date="$(date +%x)" # get date (in local format)
    echo "$time | $date | Hello world!" # print data: time | date | Hello world!
    sleep 1
done

```

<sub>Written by [vosjedev](https://vosjedev.pii.at/)</sub>
