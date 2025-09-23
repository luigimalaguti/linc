import { Hono } from 'hono';
import type { Log } from './inMemory';
import { inMemory } from './inMemory';

const app = new Hono();

app.get('/api/health', (context) => {
    return context.json({
        status: 'ok',
        timestamp: new Date().toISOString(),
    });
});

app.get('/api/logs', (context) => {
    const limit = Number(context.req.query('limit')) || 10;
    const offset = Number(context.req.query('offset')) || 0;
    const level = context.req.query('level') || undefined;
    const module = context.req.query('module') || undefined;

    const logs = inMemory.filter((log) => {
        if (level && log.level.toLowerCase() !== level.toLowerCase()) {
            return false;
        }
        if (module && log.module_name.toLowerCase() !== module.toLowerCase()) {
            return false;
        }
        return true;
    });
    const total = logs.length;

    return context.json({
        total,
        limit,
        offset,
        logs: logs.slice(offset, offset + limit),
    });
});

const parseLog = (rawLog: string): Log => {
    const jsonLog = JSON.parse(rawLog);
    return {
        timestamp: new Date(jsonLog.timestamp).toISOString(),
        level: jsonLog.level,
        thread_id: jsonLog.thread_id,
        module_name: jsonLog.module_name,
        filename: jsonLog.filename,
        line: Number(jsonLog.line),
        func: jsonLog.func,
        message: jsonLog.message,
        raw: rawLog,
    };
};

app.post('/api/logs', async (context) => {
    const raw = await context.req.text();

    const log = parseLog(raw);
    inMemory.push(log);

    return context.json({ log });
});

export default {
    port: process.env.PORT || 3000,
    fetch: app.fetch,
};
