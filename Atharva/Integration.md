Although each module is standalone, together they simulate a complete city-level digital ecosystem.
The architecture flows naturally from data collection → logistics → market systems → inspection → recalls.

High-Level Flow
Data → Logistics → Market → Inspection → Recalls


Each block is supported by one or more algorithmic modules.

Module Interactions (Detailed)
Case 1 — Weather (Boyer–Moore)

Influences agricultural and production decisions.

Predicts rainfall patterns.

Supports:

Case 3 (BST Expiry) — adjusting expiry schedules based on storage risk

Case 5 (Fenwick Pricing) — crop supply changes → pricing shifts

Case 2 — Trie Dictionary

A universal lookup system used across many modules.

It indexes:

product names

seeds

tech-repair parts

co-working services

digital business entities

This is a non-code dependency shared conceptually by the whole system.

Case 3 — Expiry Tracking (BST)

Manages perishable items’ expiry timelines.

Feeds directly into:

Case 7 (Inventory HashMap)
→ maps items to storage locations with expiry considered

Case 10 (Recall System)
→ identifies whether faulty goods are already near expiry

Case 4 — Traffic Classification (QuickSort)

Analyzes congestion patterns from IoT sensors.

Influences routing systems:

Case 8 (Floyd–Warshall)
→ adjusts delivery cost matrices

Case 9 (Dijkstra)
→ determines fastest inspection paths

Case 5 — Market Pricing (Fenwick Tree)

Tracks continuous price fluctuations.

Used in:

Case 8 (Distribution Planning)
→ some deliveries prioritize cheaper or high-demand goods

Case 10 (Recall Detection)
→ sudden price spikes can indicate defective product batches

Case 6 — Warehouse Placement (Kruskal MST)

Builds optimal storage network across city regions.

Used by:

Case 7 (HashMap Inventory)
→ stores product-to-location mappings

Case 8 (Floyd–Warshall)
→ shortest path graph is built on top of MST layout

Case 7 — Inventory Mapping (Hash Table)

Provides O(1) product → warehouse lookup.

Feeds:

Case 9 (Inspector Dijkstra Routes)
→ inspectors quickly locate target warehouse

Case 10 (Rabin–Karp Recall)
→ recall system immediately identifies which warehouse holds a bad batch

Case 8 — Distribution Optimization (Floyd–Warshall)

Computes shortest distance between all city nodes.

Used as:

Backbone for multi-warehouse → multi-shop routing

Precomputed cost matrix for:

Case 9 (Dijkstra Inspector Routing)

Logistics dashboards

Case 9 — Inspector Routing (Dijkstra)

Day-to-day pathfinding tool.

Used for:

food inspectors

IoT maintenance teams

drone checks

tech center inspections

Supports:

Case 10 (Recall)
→ ensures recalled products are collected via minimum-cost path

Case 10 — Recall System (Rabin–Karp)

Locates faulty batches via substring pattern search.

Relies on:

Case 7 (HashMap Inventory)
→ to find warehouse holding bad batch

Case 9 (Dijkstra)
→ computes quickest route for recall pickup

Case 3 (BST Expiry)
→ determines urgency if the batch is near expiry
