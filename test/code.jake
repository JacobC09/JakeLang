var choice = input("Rock (r), Paper (p) or Scissors (s)?: ");
var computer = random(0, 2);

if (computer == 0) {
    print "Computer chose rock";
} 

if (computer == 1) {
    print "Computer chose paper";
}

if (computer == 2) {
    print "Computer chose scissors";
}

if (
    choice == "r" and computer == 2 or
    choice == "p" and computer == 0 or
    choice == "s" and computer == 1
) {
    print "You Won!";
} else if (
    choice == "r" and computer == 0 or
    choice == "p" and computer == 1 or
    choice == "s" and computer == 2
) {
    print "Its a tie!";
} else {
    print "You lost :(";
}