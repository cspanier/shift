# shift.rc - Resource Compiler Library

This library implements the resource compiler core and available actions.

## Motivation

Most resource file formats are not optimized for use in realtime 3D applications. Some formats store information in data structures that are optimized for editing (e.g. shaders stored in textual source code format) or that don't match those required by graphic cards (e.g. meshes with index buffers per vertex component), while other formats lack information required for rendering (e.g. mipmaps in images). Thus, it is beneficial to convert such files into different formats that better suit our needs.

## Overview

The base abstraction for all available file transformation functions is an `action`. Each `action` operates on one or multiple input files and writes one or multiple output files. Because there are thousands of resource files to work on, it is impractical to explicitely write down each and every action with its inputs and outputs. Instead, there is a more generic `rule` type, which describes inputs using regular expression patterns, and outputs using paths that contain variables, which get replaced during processing. All `rule`s are read from JSON files from the resource compiler's input folder. The resource compiler creates a `job` for each set of input files that match a `rule`'s inputs. An output of one `job` may be the input for another `rule`. Because output paths are sometimes only known after `action` processing, all rules are grouped in passes. The timestamp and hash of each `job`'s input and output files are cached along with the assotiated `action`'s version. Thus, subsequent resource compiler invocations may skip unnecessary work. All `job`s may be run in parallel, unless the `job`'s `action` explicitely disallows parallel invocation. Because some `action`s are chained, the resource compiler has not only an input and output folder, but also a build folder containing all temporary intermediate files.

## Rules

The default filename of files containing rules to search for is `.rc-rules.json`. The format of such files is as follows:

```JSON
{
  "<rule-name>": {
    "pass": <integer>,
    "action": "<action-name>",
    "input": {
      "<input-name>": "<input-pattern>"
    },
    "group-by": [ 1 ],
    "output": {
      "<output-name>": "<output-path>"
    },
    "options": {
      "<option-name>": <value>
    }
  }
}
```

Each file may contain many rules and each input, output, and options section may contain many entries.

* `rule-name` must be globally unique among all rules in all rule files.
* `pass` is assigned an integer from 1 to n. Actions with lower pass numbers are processed first.
* `action-name` must be a name of one of the available actions.
* `input` defines a set of input slots.
* `input-name` must be a name of an input slot provided by the previously selected action.
* `input-pattern` may contain a [regular expression](https://en.wikipedia.org/wiki/Regular_expression) using the [modified ECMAScript grammar](https://en.cppreference.com/w/cpp/regex/ecmascript). It may also contain capture groups using parentheses, which may be referenced from `output-path` or `group-by`.
* `group-by` is an array of `input-pattern` capture group IDs used to group all matching files across all input slots whose capture groups with the given IDs are equal into single jobs. This array is empty by default.
* `output` defines a set of output slots.
* `output-name` must be a name of an output slot provided by the previously selected action.
* Both `input-name` and `output-name` are not arbitrary, but must be looked up in the action's documentation.
* `output-path` is a path to store the action's result to. It may contain capture group references in the form of `<input-name:id>`. `input-name` matches the pattern's input slot name and id is the number of the capture group in the pattern starting with 1.
* `option-name` must be a name of an option provided by the previously selected action. Some actions provide configuration options.
* Both `input-pattern` and `output-path` may contain variables in the form of `<variable-name>`. These variables must exist and are replaced with their value before interpretation.

## Global Variables

* `<input-path/>`: the absolute path to the input folder specified using `void resource_compiler::input_path(const fs::path& value)`, including a trailing slash.
* `<build-path/>`: the absolute path to the build folder specified using `void resource_compiler::build_path(const fs::path& value)`, including a trailing slash.
* `<output-path/>`: the absolute path to the output folder specified using `void resource_compiler::output_path(const fs::path& value)`, including a trailing slash.
* `<rule-path/>`: The path to the current rules file relative to `<input-path/>`, including a trailing slash. If the current rule path is equivalent to `<input-path/>` this variable expands to an empty string instead.

## Available Actions

* group_resources
* image_import
* mesh_export_obj
* mesh_import_ply
* mesh_tootle
* font_import_ttf
* shader_compile
* scene_compile
* scene_import_gltf
* scene_import_pbrt

### Example:

```JSON
{
  "textures-engine-import": {
    "pass": 1,
    "action": "image-import",
    "input": {
      "image": "<input-path>/<rule-path>/(.*)\\.(png|jpg|tif)$"
    },
    "output": {
      "header": "<build-path>/<rule-path>/<image:1>.image_header",
      "buffer": "<output-path>/<rule-path>/<image:1>.lod_<lod-level>.image_buffer"
    },
    "options": {
      "target-format": "rgba8_srgb",
      "normalized": false
    }
  }
}
```
This rule imports texture images in PNG, JPEG, or TIFF format and stores the image header information in the build folder and different mipmaps in the output folder. While `<input-path>`, `<build-path>`, `<output-path>`, and `<rule-path>` are global variables, `<image:1>` references the first capture group in the input `image`, which is the resolved value of `(.*)`. `<lod-level>` is an action specific variable.
