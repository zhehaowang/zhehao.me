// template definition usually goes in the header, as the alternative is to
// instantiate explicitly in template.cc (note not client.cc!). The reason
// for that is upon seeing the explicit instantiation in template.cc, compiler
// is forced to generate impl functions for that instantiation, so later on
// when client uses it linker can pick up from functions already generated for
// that instantiation.
// Putting this instantiation in client header doesn't help as when compiling
// client's compilation unit, compiler could have already missed generating
// concrete functions for the specialization client wants.

template <typename T>
class MyTemplate {

};