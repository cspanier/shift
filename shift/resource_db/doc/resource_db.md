# shift.resource_db - A Graphic Resource Library

The purpose of `shift.resource_db` is to efficiently store data that is commonly used in graphics applications.

* Quickly provide information about dependencies between resources
* Use IDs instead of paths for cross references

Each resource is assigned an identifier that is computed from a hash of its data. Cross references between resources are not 

* [buffer](#buffer)
* [buffer_view](#buffer_view)
* [font](#font)
* [image](#image)
* [material](#material)
* [material_descriptor](#material_descriptor)
* [mesh](#mesh)
* [resource_group](#resource_group)
* [scene](#scene)
* [shader](#shader)

### buffer
A buffer is nothing more than an array of bytes. It contains data that is usually loaded directly to device memory.

### buffer_view
A lightweight object referencing a subrange of a `buffer`.

### font
(ToDo)

### image
Compared to most other image formats our format splits header information (size, format, ...) from pixel data. This way headers of all images can be loaded more efficiently. This also allows packing together all image header structures into a single `resource_group`.

### material
(ToDo)

### material_descriptor
(ToDo)

### mesh
(ToDo)

### resource_group
A container resource that groups together `image` and `mesh` header information into a single resource.

### scene
(ToDo)

### shader
(ToDo)
