# Realism rules for procedural dressing

The look comes from **verisimilitude**: things placed and connected the way the real world
would place them. A believable layout survives an art swap; polished meshes in a nonsensical
layout never read well. So review **sense first, finish last**.

Use these to appraise any PCG tool's output (and any hand-dressed area) before a person has to.
For each rule: can the bake check count it? If yes, it becomes a rule with a count. If not, it is
a checklist item the reviewer answers per shot.

| Rule | The question | Typical failure | Measurable as |
|---|---|---|---|
| **Support** | Is every piece held up by something that could hold it? | A brazier floating a hand's width above a ledge | Gap between a piece's foot and the surface below |
| **Clearance** | Does it fit where it is, without passing through anything? | A banner pole through a window; armour stands clipping a wall | Overlap of occupancy volumes |
| **Connection** | Does every run start and end somewhere real? | A chain or pipe that ends in mid-air; a gutter with no downpipe | Run ends not on a connector or fitting |
| **Orientation** | Does it face the way its use needs? | A torch bracket facing the wall; a bench facing a blank corner | Angle between a piece's front and its host's |
| **Access** | Could someone reach, open or use it? | A door blocked by crates; a ladder to nowhere | Keep-clear volumes intruded |
| **Context** | Would this be here, in this district, at this height? | A market stall on a rooftop; a gate winch inside a tavern | Allowed-zone tags per piece |
| **Edges** | Are joints, corners and ends finished? | A wall run that stops without a pier; a railing with no end post | Run ends without an end piece |
| **Repetition** | Does the eye catch a stamp? | The same cracked tile every third bay | Same variant adjacent, or the same seed per run |
| **Keep-out** | Does anything intrude on space that must stay clear? | Debris in the combat area; a beam across the third-person camera's arc | Intrusions into never-cross volumes |

## How to apply

- Every PCG job brief carries these rules and asks for an appraisal of its own captures
  against them before reporting done.
- **A tool that cannot satisfy a rule drops the piece.** A missing lantern is invisible; a
  floating one is the first thing a player sees.
- **Prefer more small purpose-built tools** (a stairwell, a railing run, a lean-to, a cable span)
  over one scatter that ignores context. Each small tool knows its own rules.
- When a fix needs a piece the kit lacks (a corner, an end cap, a bracket), make the piece in
  the DCC rather than faking it in the graph or dropping the fix.
- Add the rules you find. Every note the owner makes by eye is a rule that was missing from this
  list.
