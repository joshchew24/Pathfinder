# Making New Levels
`-1.json` is a sample level containing all the attributes listed below. Feel free to use it as a template, but it may not be up to date with any new features added.
## Required Attributes
The level manager expects these attributes to be in your level. If they're missing, you'll get an error. Replace `<x>` and `<y>` with the coordinates of the associated entity.

1. `level_end: [<x>, <y>]`
2. `player_spawn: [<x>, <y>]`

## Entity Array Schemas
If you choose to use one of these entities in your level, the level manager expects the contents of the entity arrays to look something like this. 
- attribute order does not matter.
- you can have multiples of all of these entities with three exceptions
    - `spawners` and `enemies` are just contains for the the actual entity arrays
    - `chase_boulder` uses A* pathfinding, so having multiple would be computationally expensive

### Schemas
1. `walls`
```json
"walls": [
    {
      "x": 1000,
      "y": 940,
      "width": 1400,
      "height": 400
    }
]
```

2. `stairs`
```json
"stairs": [
    {
      "quantity": 10,
      "x": 800,
      "y": 250,
      "gap": 50,
      "width": 100,
      "height": 20
    }
]
```

3. `spikes`
```json
"spikes": [
    {
        "quantity": 46,
        "x": 100,
        "y": 503,
        "gap": 40,
        "angle": 180
    }
]
```

4. `hints`
```json
"hints": [
    {
      "npc": [ 750, 702 ],
      "text": "Hello, I am the hint guy, I will give you a hint on certain levels! For now, just follow the tutorial and reach the trophy",
      "text_pos": [ 550, 440 ]
    }
]
```

5. `spawners` contains `boulder_spawners` and `spike_projectile_spawners`
```json
"spawners": {
    "boulder_spawners": [
      {
        "x": 1000,
        "y": -100,
        "random_x": false,
        "delay": 6000
      }
    ],
    "spike_projectile_spawners": [
      {
        "x": 1900,
        "y": 553,
        "width": 40,
        "height": 20,
        "x_vel": -600,
        "y_vel": 0,
        "angle": -90,
        "delay": 3500
      }
    ]
}
```

6. `enemies` contains `chase_boulder` and `archers`
```json
"enemies": {
    "chase_boulder": [ 1000, 100 ],
    "archers": [
      {
        "x": 1000,
        "y": 1000
      }
    ]
}
```

7. `paintcans`
```json
"paintcans": [
    {
      "x": 1000,
      "y": 1000,
      "fixed": false
      "value": 100
    }
]
```

## Singular Optional Attributes
### Entities
1. `"checkpoint": [ <x>, <y> ]`
### Attributes
You don't have to include these in your file. Excluding them or setting them to `null` will take the default value from `config.json`
1. `"gravity": <float>` - overrides gravity
2. `"friction": <float>` - overrides friction
3. `"ink_limit": <float>` - overrides ink limit
4. `"line_collision_on": <bool>` - toggle for player physics interactions with lines
5. `"disappearing": <bool>` - toggle for causing all walls/platforms to disappear after timer expires
6. `"disappearing_timer": <float (ms)>` - override for timer above
7. `"mouse_gesture": <bool>` - toggle for a currently hard-coded feature. renders shape outlines for player to draw to procure platforms. recommended for use with `"line_collision_on": false

## Extending Functionality
The level manager will ignore attributes it doesn't expect to see in a level's JSON file. This is helpful in two ways:
1. adding new features to new levels won't break old ones
2. in large, complex levels, you can add `"name"` or `"id"` attributes to objects for clarity without breaking anything (as long as they don't collide with existing keywords)

It is easy to create custom level features and add them to the level manager for others to reuse. Here's an example:

### Example
TODO: add example