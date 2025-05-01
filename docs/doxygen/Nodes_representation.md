# Nodes Representation

## Representation in scene

| Field         | purpose                 |  calculation source           |  modification                  |
| -----------   | ----------------------- | ----------------------------- | ------------------------------ |
| id            | unique per scene        | on creation, from DB          | not possible after assignment  |
| position      | location in scene space | on creation, from input       | by user interaction            |
| width,height  | determines dimensions   | on creation, from class       | by user interaction            |
| node class    | see notes               | on creation, from template    | not possible                   |
| styler        | draws node and sockets  | on creation, from template    | maybe possible                 |
| socket counts | connections             | from class and properties     | only through other properties  |
| properties    | per node data store     | set by class spec by template | by user through prop tab       |

**note 1:** class purpose is to define the properties, and it is used along with the properties to calculate the socket counts.

**note 2:** the only point when sockets counts need recalulcation is when the properties change, the class should calculate the number of sockets.

**note 3:** the styler calculates the node positions.

## nodes template
 
| Field                | purpose                 | source             | example         |
| -------------------- | ----------------------- | ------------------ | --------------- |
| name                 | human readable name     | spec file          | My Add          |
| source               | source file and type    | calculated on load | myadd.txt       |
| class source         | see notes               | spec file          | myaddplugin/add |
| default properties   | list of default prop    | spec file          | in_ports = 3    |
| default width,height | should be clamped       | spec file          | 10, 20          |
| default styler       | draw nodes and sockets  | spec file          | add_styler      |

**note 1:** class source should specify the plugin and class names, but not where they are to be loaded from for security reasons, keep the plugin loading separate, and only user user supervision. 

**note 2:** when class is loaded the default properties needs to be verified against the class.