var set;
var get;

func a() {
    var value;

    func setValue(number) {
        value = number * 2;
    }

    func getValue() {
        return value;
    }

    set = setValue;
    get = getValue;
}

a();

if (hi) {
    print 2 * 2;
}

loop {
    print hi;
}

while (jake == 2) {
    print hi;
}

jake.prop = 2;


set(4);
print get();
