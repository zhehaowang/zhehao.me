$1 > 0.0 {r = r + 1}
END {
    if (r > 0) {
        printf("matching line count %d\n", r);
        printf("we good\n");
    } else
        printf("no matching line")
}