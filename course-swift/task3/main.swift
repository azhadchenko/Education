import Foundation


protocol DictionaryCoding {
    init(dictionary: [String : AnyObject]) throws
    var dictionary:  [String : AnyObject] { get }
}

enum Level {
    case Public, Private, Working, NotStated
}

enum EmailError : ErrorType {
    case NilInput
}

struct Email : DictionaryCoding {

    let email: String
    var level: Level

    var dictionary: [String : AnyObject] {
        return ["email" : self.email, "level" : self.level]
    }

    init(dictionary: [String : AnyObject]) throws {

        if let email = dictionary["email"] { self.email = email as! String }
        else { throw EmailError.NilInput }

        if let level = dictionary["level"] { self.level = level as! Level }
        else { throw EmailError.NilInput }
    }
}

do{
    let example = try Email(dictionary: [:])
} catch EmailError.NilInput {
    print("Nil input")
} catch {
    print("Thats weird")
}



do{
    let example = try Email(dictionary: ["email" : "demens@parallels.mipt.ru", "level" : Level.Working])
    print("this: \(example.dictionary)")
} catch EmailError.NilInput {
    print("Nil input")
} catch {
    print("Thats weird")
}
