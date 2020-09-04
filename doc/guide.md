# ▀▄ Monotty Desktop

## Usage Guide

### tl;dr: [`Creation`](#creation)

## Description

Monotty desktop environment is designed to organize the simultaneous operation of many text applications and services in a shared virtual text space.

The term "virtual text space" means a text mode in the conventional sense, but having a geometrically unlimited area for text output by applications running in this environment, and not limited to the visible screen or terminal emulator window. The environment has a user-friendly interface that allows users to easily navigate among the running applications in this virtual workspace.

At its core, this desktop environment is a graphical user interface gracefully projected onto classic text mode, allowing us to combine the convenience of a GUI with the lightness of a text-based UI. This projection is possible using TrueColor and Unicode text. Coloring of symbols allows us to outline the boundaries of objects, and the variety of Unicode characters - to depict the semantic meaning of interface elements.

The main advantage of the text UI is its simplicity and resource efficiency, allowing it to be used where it is impractical or impossible to use graphics. This desktop environment is designed with an emphasis on keeping this value at the required level.

## Interface

All visible elements can be active, including any text fragment, that is, they can respond to user actions using the keyboard, mouse, etc.

Desktop Components
- _Viewport_ - the visible part of the screen or terminal emulator window.
- _Window_ (object)
  * _Molding_ - a frame around the window with a size of one cell.
  * _Interior_ - the content of the object.
- _Object list_ - a list of titles located in the upper left corner of the viewport, it is always in the background.
- _List of users_ - a list of users sharing the environment.
- _Navigation string_ - the line connecting the object to the center of the viewport, it is always in the background.
- _Debug overlay_ - debug information displayed over the viewport, it is always in the foreground, but it is transparent to mouse actions.

The following mouse actions are handled by the environment:
 - button press
 - button releasing
 - button click (pressing and releasing)
 - cursor moving
 - hovering over an object
 - cursor entry
 - cursor leaving
 - dragging (moving the cursor with pressed buttons)
 - wheel scrolling

All of the above actions performed simultaneously by several buttons are registered and processed by the environment as actions of a certain virtual button, represented as a combination of those involved, for example, an action to drag with simultaneously held down the left and right buttons, are interpreted by the environment as an action to drag and drop, performed by the virtual button `Left+Right`.

### Creation <a name="creation"></a>

The type of objects to create must be chosen in the menu. This is done with the _left_ mouse button.

The object of the selected type will be created in the area marked by dragging with the right or middle mouse button over the viewport. The object will be created as soon as the button is released.
 
_Note_
 - To cancel, you can click the left mouse button while dragging, or reduce the selection area to zero sizes.
 - Dragging with the middle button is not captured by anything and can be dragged over other objects, unlike the right button, dragging which can be captured by the object that is currently under the mouse cursor.

### Destruction

To delete an object, just click on it with the _medium_ mouse button.

Also, the object will be deleted by clicking with the middle mouse button on any element of the interface associated with the object, for example, the title in the list of objects or the navigation string associated with the object.

### Resizing

Changing the size of an object can be done by dragging with the _left_ button any area of the object near its edge.
 
_Note_
 - The direction of resizing is highlighted on the molding around the object.
 - If the object captures all mouse events, then resizing is possible only with molding around the window.
 - If the button is released while moving, the object will begin to move, consuming the received kinetic energy.

### Movement

Moving an object can be done by dragging with the _left_ button any area of the object near its center.
 
_Note_
 - To distinguish movement from resizing, you can focus on the molding around the object.
 - If the button is released while moving, the object will continue to move, consuming the received kinetic energy.

### Panoramic navigation

Moving the viewport can be done by dragging the _left and right_ buttons simultaneously.
 
_Note_
 - If the buttons are released while moving, the viewport will continue to move, consuming the received kinetic energy.

### Group movement

Synchronous movement of all visible objects is possible by dragging the left mouse button over any free area of the viewport.
 
_Note_
 - The movement of objects is limited to the viewport so that they cannot leave it.
 - If the button is released while moving, the objects will continue to move, consuming the received kinetic energy.

### Following

To move the viewport to the location of the object, click the _left_ mouse button on any interface element associated with the object, for example, the title in the list of objects or the navigation string associated with the object.

### Magnetize

To move an object to the current mouse position, click the _right_ mouse button on any interface element associated with the object, for example, the title in the list of objects or the navigation string associated with the object.

### Magnetize (menu)

To move the menu window to the current mouse position, click the _right_ mouse button on any free area.

## Multi-seat architecture

The desktop environment is designed to be shared by any number of users.

For each connecting user/group ID, a unique instance of the environment is created. If the environment instance for the connecting user is already running, the user will be connected to it. User identification is performed at the kernel level of the operating system by checking the user/group ID at the ends of the channel between the environment and the connecting user. Thus, multiple users can be connected to the shared workspace using a common user ID or belonging to a specific group.

The desktop input controller processes input device events from all participants in the order in which they are received and provides connected users with shared access to all interface elements.

Users can disconnect from the current instance of the desktop environment at any time, the current instance will continue to run regardless of whether there are any connected users, so users can reconnect to that instance at any time.

Any connected user can shutdown the current instance of the environment, and all connected users will be disconnected from it automatically.

...
