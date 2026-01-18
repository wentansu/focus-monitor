import UIKit

@available(iOS 15.0, *)
class SmartSpectra {
    
    func ScreeningPage(recordButton: Model.Option.Button.Record? = nil) -> ViewController.Screening.Root {
        return createScreeningPage(option: recordButton)
    }

    internal func createScreeningPage(option: Model.Option.Button.Record? = nil) -> ViewController.Screening.Root {
        
        let propertyObject = RecordButtonPropertyObject.init(recordButton: option)
        let vm = ViewModel.Screening(propertyProvider: propertyObject)
        let vc = ViewController.Screening.Root(viewModel: vm)
        return vc
    }
}
