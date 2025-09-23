type Log = {
    timestamp: string;
    level: string;
    thread_id: string;
    module_name: string;
    filename: string;
    line: number;
    func: string;
    message: string;
    raw: string;
};

const inMemory: Log[] = [];

export type {
    Log,
};
export {
    inMemory,
};
