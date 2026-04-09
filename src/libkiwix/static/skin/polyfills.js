// A few browsers do not support the use of String.prototype.replaceAll method.
// Hence we define it once we verify that it isn't supported. For documentation
// see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/replaceAll
if (!String.prototype.replaceAll) {
    String.prototype.replaceAll = function (pattern, replacement) {
        // verify parameter: It must either be a string or a RegExp with a global flag.
        if (typeof pattern[Symbol.replace] === 'function') {
            // the pattern is a RegExp check for the presence of g flag.
            if (pattern.global) {
                return this.replace(pattern, replacement);
            } else {
                throw new TypeError('Global flag for regular expressions')
            }
        }
        // the pattern is not a RegExp, hence it must be a string.
        if (typeof pattern !== 'string') {
            throw new TypeError('pattern must either be a string or a RegExp with a global (g) flag.')
        }
        return this.replace(new RegExp(pattern, 'g'), replacement);
    }
}
