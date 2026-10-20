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

## Verify per engine version

How the custom properties survive differs between export formats and translators. FBX
exports object properties only, and translator coverage changes between versions. Before
building on it, import one test asset and confirm every channel arrives: custom
properties, sockets, collision, extra UV sets and named material slots. Let that test pick
the format.
