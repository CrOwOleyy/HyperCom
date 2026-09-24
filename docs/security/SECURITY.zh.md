# 安全政策

[English](../../SECURITY.md) · [Français](SECURITY.fr.md) · **中文** · [हिन्दी](SECURITY.hi.md) · [Español](SECURITY.es.md) · [العربية](SECURITY.ar.md) · [বাংলা](SECURITY.bn.md) · [Português](SECURITY.pt.md) · [Русский](SECURITY.ru.md) · [日本語](SECURITY.ja.md)

Hypercom 由一个小团队维护,不在任何专业机构框架之内。这份文档如实
说明这意味着什么。

## 报告漏洞

**不要为安全漏洞开一个公开的 issue。** 请使用本仓库的
[GitHub 私密漏洞报告](../../../../security/advisories/new)
功能(*Security* 标签页 → *Report a vulnerability*):在修复发布之前,
报告只对维护者可见。

请描述:

- 涉及的文件和函数(如果可以确定);
- 复现问题的确切条件;
- 具体影响(攻击者能得到什么,而不只是"这看起来不对")。

## 属于范围内的问题

任何让 [docs/THREAT_MODEL.md](../THREAT_MODEL.md) 中列出的某项保证
失效的漏洞——例如:在没有收件人密钥的情况下能读取私信、绕过基于
签名的身份验证、SQL 注入、无需身份验证即可远程触发的崩溃、内存中
泄露的密钥。

## 不算漏洞的情况

`THREAT_MODEL.md` 记录的是**接受**的限制,不是遗漏:服务器能看到
谁在给谁写信,"仅好友可见"不是加密,恶意的服务器运营者可以对自己的
用户撒谎。针对这些具体问题的报告会被关闭,并指向该文档,而不会
当作 bug 处理。

## 受支持的版本

Hypercom 目前还没有带编号的稳定版本:只有 `main` 分支会收到安全
修复。

## 响应时间

没有正式的 SLA——这是一个两人项目,不是一家公司。实际情况是:一周内
确认收到,在报告公开之前会告知修复方案或行动计划。
