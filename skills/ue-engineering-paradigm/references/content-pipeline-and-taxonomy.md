# Content pipeline and taxonomy: extend the engine's pipeline, don't build beside it

When a content family grows (environment pieces, weapons, armour, props), there are two
pressures: getting assets **in** cheaply and correctly, and **finding** them again later.
The reflex is a bespoke tool: a custom importer, a sidecar database, a browser panel, and a
naming scheme that packs facts into asset names. Every one of these duplicates something
the engine already does and then drifts away from it.

**The rule: one shared vocabulary, carried by the engine's own machinery, from the DCC to
the Content Browser.** The code you write is a thin layer that makes each engine stage
speak that vocabulary. It is not a parallel system.

## The chain

| Stage | Engine provides | You add |
|---|---|---|
| Authoring (Blender, Maya...) | custom properties on objects, exported by FBX (user properties) and glTF (extras) | a naming rule for a few properties (`<family>_form`, `<family>_style`...) |
| Import | Interchange translators surface those as user-defined attributes (`UInterchangeUserDefinedAttributesAPI`) on the translated nodes; pipeline stages run before and after the factories | one `UInterchangePipelineBase` subclass that checks the attributes against the vocabulary and copies them onto the asset |
| Storage | asset user data on the asset; asset registry tags (`GetAssetRegistryTags`, `AssetRegistrySearchable`) | one small `UAssetUserData` class that surfaces a few keys |
| Vocabulary | gameplay tags: hierarchical, ini-defined, parent matches children | one taxonomy ini per family |
| Finding | Content Browser text search, `Key=Value` tag queries, front-end filter extensions | one filter extension whose menus are built from the taxonomy |
| Guarding | Data Validation (`UEditorValidatorBase`) | one validator: cardinality, prefix, contract |
| Placing | PCG (attribute-driven selection, grammars), the Asset Manager | selectors keyed on the same tags |

Each "you add" cell is small, and each engine upgrade improves the stage underneath it
instead of breaking a tool you wrote beside it.

## The asset describes itself

Per-asset facts travel **inside the source file** as DCC custom properties, and a reimport
refreshes them. Benefits:

- **No sidecar to drift.** A manifest or spreadsheet next to the meshes is a second store
  of the same fact (the decision procedure's step 1). Someone renames or adds a variant and
  the two disagree silently.
- **Import is classification.** The asset arrives tagged, searchable and placeable. No one
  types the facts again into a details panel.
- **Any author, the same route.** A hand-modelled one-off or an outsourced piece is tagged
  in the DCC and lands exactly like a generated one.
- **The checks happen on the way in.** An unknown tag or a missing required facet fails the
  import with a clear message. This is "validate at the edge" applied to the content
  pipeline.

Keep in the asset only what **nothing else records**:

- Size is in the engine's bounds, sockets are on the mesh, and the source file is in the
  import data.
- Which collection a piece belongs to is its plugin or folder path.
- Selection weights belong to the set that selects (a kit or recipe data asset), because
  sets override them.
- A *declared* value that deliberately differs from a derived one (an authored module
  length versus the bounds of an overhanging piece) does belong in the asset.

## Facets, not names

Split the vocabulary into independent **facets**, each a tag branch with a cardinality:

- **Form**: what it is (`Wall.Panel`, `Weapon.Blade.Short`). Exactly one. It doubles as the
  slot the placing system fills.
- **Style**: how it looks (`Aesthetic.Futuristic`, `Wear.Damaged`, `Surface.Metal`). One or
  more.
- **Trait**: behaviour flags (`Tall`, `Emissive`, `Stackable`). Zero or more.
- Numbers (length, weight) are typed values, not tags, so they support ranges.

Put **aesthetic style on the asset, not on its collection.** "Every futuristic wall variant"
is a search *across* collections. If style only lives on the kit, that search is
impossible, and a kit can never mix styles.

With facets in the registry, **names can stay human**: a type prefix plus a short name
(`SM_DentedPanel`). The same short name in two plugins is fine; a search finds both. A name
like `SM_Yard_BoundaryPanel_Dented_300` is a registry squeezed into a string. It cannot be
queried by range, it breaks on every rename, and it lies the moment the asset changes.

A good filter is the facets as menus: any ticked tag within a facet matches, every facet must
match, ticking a parent includes its children, and the menus are built from the taxonomy
file so a new tag needs no code.

## Content organised as plugins

- **Generic libraries** (kits) are content plugins that know nothing about their users.
- **Specific content** (a level, a boss arena, a hero weapon set) is its own plugin, often a
  Game Feature Plugin, that **depends on** libraries and appends to them: it adds variants,
  overrides a slot, or adds one-off pieces that fill no slot. Resolution order: library
  defaults, then the dependent's adds and overrides, then its one-offs.
- Registered plugins are in the asset registry before they load, so a catalogue lists them
  without hard references.

## The contract is the interface

Generated greybox (a DCC script that builds a fortress kit's walls, towers and battlements)
gets you a playable arena fast. Later a person replaces pieces with hand-made art. That swap
works only if **everything the placing tools read comes from a contract the piece declares**,
never from the generator: not its naming quirks, not offsets it happens to bake in, not a
lookup keyed on how it built the mesh.

The contract, per piece:

| Part | What it says | Notes |
|---|---|---|
| **Frame and pivot** | Units, axes, and where the pivot sits for this form | Per form: a wall run centred on its length and standing on zero; a fitting on its joint centre; a prop at bottom-centre. Write the rule per form, not per piece |
| **Declared length** | The module size the tools tile by | Checked against the mesh to a tolerance; a mismatch gaps or overlaps every run |
| **Tags** | One form, one or more styles, its traits (the facets above) | An unknown tag stops the import |
| **Material slots** | A fixed slot order by role (body, team paint, emissive, glass, accent) | A slot that owns no face may be dropped on import; overlays stand proud so no two slots are coplanar |
| **Sockets** | Mounts (where another piece goes: a torch bracket, a banner pole) and connectors (where a run joins: a chain, a rope, a drainage pipe) | A connector carries its direction, size and what it carries as custom properties. A fitting has one on every mating face, pointing out along its arm |
| **Volumes** | Occupancy (the solid), keep-clear (a door's swing, a portcullis track), walkable tops, keep-out classes, no-build, decal-paintable faces | Named boxes in the DCC, or authored in the engine's mesh editor |
| **Collision** | For the player and physics only | Tools read occupancy volumes, never collision |

**Occupancy is a few tight boxes, one per real part, never one bounding box.** A balcony is its
slab and each railing run with the deck open; a stair is its flights in steps. The failure this
rule comes from: a generator that fell back to a single bounding box whenever a piece had more
parts than its cap. The router then treated whole balconies and stairways as solid, every gap a
chain or pipe could run through was blocked, and it took a voxel audit to find out why routes
detoured. The same audit found the router using a piece's coarse **collision** as a routing
surface whenever its occupancy was sparse.

**How a hand-made piece gets in:**

- The artist authors the contract in the DCC (named empties for sockets and volumes, custom
  properties for tags and connector data), and the import reads it exactly as it reads a
  generated piece's.
- Or the contract is added in the engine (a mesh-editor tool for volumes and sockets, stored so
  it survives reimport).
- Anything missing is **derived from the mesh with a safe default** (tight multi-box occupancy,
  never the bounds) and **flagged by data validation**, so the gap is visible to the artist
  rather than guessed by PCG.
- Keep a short artist-facing page of what a piece must carry. The config files are the
  authority; the page summarises them and says so.

Fixture levels that test the placing tools against these declarations: the `ue-pcg-fixtures`
skill.

## Red flags

| You see | It means | Do instead |
|---|---|---|
| Asset names encoding kit, slot, variant, size | A registry squeezed into a string | Facets in the registry; short human names |
| A CSV or JSON manifest that must match the meshes | A second store that will drift | Properties inside the source file; the manifest keeps only set-level data |
| A custom import tool or editor importer for one family | A parallel pipeline the engine will outgrow | An Interchange pipeline asset |
| A custom asset browser window | A second browser beside the Content Browser | A front-end filter extension |
| Free-text tags typed per asset | A vocabulary with no dictionary | A gameplay-tag taxonomy file, checked at import |
| A style or aesthetic stored only on the collection | Cross-collection searches are impossible | The facet on each asset |
| Size or sockets copied into metadata | Duplicated engine data | Read the bounds and sockets |
| A placing tool that reads a generator's names or offsets | Hand-made art cannot replace the greybox | Read only the declared contract |
| One bounding box as a piece's occupancy, or collision used for routing | Every gap a run could use is blocked | Tight multi-box occupancy; tools read occupancy only |

## Verify per engine version

How the custom properties survive differs between export formats and translators. FBX
exports object properties only, and translator coverage changes between versions. Before
building on it, import one test asset and confirm every channel arrives: custom
properties, sockets, collision, extra UV sets and named material slots. Let that test pick
the format.
