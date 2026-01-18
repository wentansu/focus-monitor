//
//  SerialTaskRunner.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 4/29/25.
//


/// A lightweight actor that executes asynchronous jobs sequentially.
internal actor SerialTaskRunner {
    private var isRunning = false
    private var queue: [() async -> Void] = []

    /// Adds a job to the queue and starts execution if idle.
    /// - Parameter job: The asynchronous closure to run sequentially.
    func enqueue(_ job: @escaping () async -> Void) {
        queue.append(job)
        if !isRunning { Task { await runNext() } }
    }

    /// Executes the next job in the queue if available.
    private func runNext() async {
        guard !queue.isEmpty else { isRunning = false; return }
        isRunning = true
        let job = queue.removeFirst()
        await job()
        await runNext()
    }
}
