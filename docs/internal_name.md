# Internal name

Files within Jactorio are referenced using internal names

**Examples:** `sand-1` | `sand-2` | `water-1`

## Format

Internal names **must not** contain spaces

Recommended character instead of a space is `-`

## Defining internal names

Defined in `data.cfg` in the directory
```
data/<unique_subfolder_name>/data.cfg
```

### Data.cfg format

```
<DATA TYPE/path/from/data.cfg/directory> = <internal-name>
```

Notice **DATA TYPE** in the above format

### Data types

The first subfolder within the data.cfg directory must be of a name listed below

They are used to let the parser know what the file will be and how to handle it

* graphics
* audio

---

**Example:** defining internal name `sand-1` in `data/terrainPlus/graphics/sand/sand.png`

*In `data/terrainPlus/data.cfg`*
```
graphics/sand/sand.png = sand-1
```
