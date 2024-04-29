; define macro with negative numbers to expand, gives errors in v1.6 and earlier
        macro pointless
        .dw -1
        .dw -2
        .dw24 -1
        .dw24 -2
        .dl -1
        .dl -2
        .db -1
        .db -2
        ld hl, -1
        ld hl, -2
        ld a, -1
        ld a, -2
        endmacro

        pointless
