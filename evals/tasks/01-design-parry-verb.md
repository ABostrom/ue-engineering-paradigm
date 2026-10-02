---
id: 01-design-parry-verb
title: Design a new "Parry" verb
exercises: SKILL.md (three layers, decision procedure), references/abilities-as-graphs.md, references/node-contract.md
---

## Prompt

We're making a third-person melee action game in Unreal 5 with the Gameplay Ability
System. We already have Dodge and Heavy Strike abilities. Now we need **Parry**: the
player presses Block; for a short window an incoming attack is *parried* (the attacker
staggers and the parrier gets a riposte opening for about a second); an attack that
lands after the window is merely *blocked* and drains stamina. Designers will want to
tune the window and costs, and they'll probably want a "perfect parry" variant later.

Design how you would build Parry. Say what goes in C++, what goes in Blueprint, and what
goes in data, and sketch the classes, functions and ability tasks you would add.

## A good answer must

- Put the **sequence** (press, wait for the window, branch on parried vs blocked, open
  the riposte) in the ability's **Blueprint graph**, with C++ supplying the nodes.
- Make the waiting a **latent `UAbilityTask`** named for what it waits on (e.g.
  `WaitForIncomingHit` / `WaitForParryWindow`), with **one exec output per outcome the
  author draws differently** (Parried / Blocked / window expired), and its cleanup in
  `OnDestroy`.
- Make instantaneous work (apply stagger, spend stamina, open riposte) **plain
  `BlueprintCallable` functions**, not tasks.
- Put the window length, stamina cost and riposte duration in **editable properties on
  the Blueprint child or a data asset**, not hardcoded in a C++ constructor.
- Treat the "perfect parry" variant as **another Blueprint child / arrangement of the same
  nodes**, not an enum or bool flag that a C++ `ActivateAbility` switches on.

## A good answer must not

- Write the whole flow in C++ `ActivateAbility` with callbacks plus member flags such as
  `bWindowOpen` / `bParried`.
- Introduce a `EParryType` (or `EDefenseType`) enum with a switch.
- Load assets by path in C++ (`LoadObject`, `ConstructorHelpers`).
- Name Blueprint-facing nodes as narration ("OnHeGotParried") or with lower-cased
  acronyms.
