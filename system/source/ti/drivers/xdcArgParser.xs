/*
 *  ======== xdcArgParser.xs ========
 */

/*
 *  ======== getDeviceWareOpts========
 *  Parses XDCARGS returning an object with paths to device *wares.
 */
function getDeviceWareOpts(xdcArgs)
{
    /* Get paths from XDCARGS options */
    var argMap = parseArgs(xdcArgs);

    return ({
        "cc26xxware": argMap["cc26xxware"],
        "cc32xxware": argMap["cc32xxware"],
        "msp432DriverLib": argMap["msp432DriverLib"],
    });
}

/* Returns a JSON map object of XDCARGS */
function parseArgs(xdcArgs) {
    var argMap = {};
    var msp432DevList = [];

    /* Parse product versions from XDCARGS options */
    for each (var arg in xdcArgs.split(" ")) {
        if (arg.split('=').length < 2) {
            /* skip undefined arguments */
            continue;
        }

        if (arg.match(/^CC26XXWARE=/)) {
            argMap["cc26xxware"] = String(arg.split("=").pop());
        }

        if (arg.match(/^CC32XXWARE=/)) {
            argMap["cc32xxware"] = String(arg.split("=").pop());
        }

        if (arg.match(/^MSP432DRIVERLIB=/)) {
            argMap["msp432DriverLib"] = String(arg.split("=").pop());
        }
    }

    return argMap;
}
