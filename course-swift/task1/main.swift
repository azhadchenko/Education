enum StudentType {
    case Botan, Freeloader, TheOneWhoDoesntCare
}

enum Department : Int {
    case DCAM = 7
    case DGAP = 2
    case DREC = 1
}

protocol Examinable {
    func passExam()
}

struct Student : Examinable {
    let firstName: String
    let seconName: String
    var type: StudentType
    var department: Department

    func passExam(){
        switch self.type {
            case .Botan                 :   print("Student \(firstName) passed the exam")
            case .Freeloader            :   print("Student \(firstName) failed the exam")
            case .TheOneWhoDoesntCare   :   print("Student \(firstName) missed the exam")
        }
    }
}

var testarray = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
let student = Student(firstName: "Andrew", seconName: "Zhadchenko", type: .Freeloader, department: .DCAM)
student.passExam()