--zadanie 1
SELECT distinct c2.CompanyName ,c2.Phone  from Customers c2
inner join orders o on o.CustomerID = c2.CustomerID 
inner join [Order Details] od on od.OrderID = o.OrderID 
inner join Products p on p.ProductID = od.ProductID 
inner join Categories c on p.CategoryID = c.CategoryID 
where c.CategoryName = 'Confections'

--zadanie 2
SELECT c2.CompanyName ,c2.Phone from Customers c2
LEFT join orders o on o.CustomerID = c2.CustomerID 
left join [Order Details] od on od.OrderID = o.OrderID 
left join Products p on p.ProductID = od.ProductID 
left join Categories c on p.CategoryID = c.CategoryID 
and c.CategoryName = 'Confections'
group by c2.CompanyName , c2.Phone , c2.CustomerID 
having count(c.CategoryName) = 0

--zadanie 3
SELECT c2.CompanyName ,c2.Phone from Customers c2
LEFT join orders o on o.CustomerID = c2.CustomerID 
left join [Order Details] od on od.OrderID = o.OrderID 
left join Products p on p.ProductID = od.ProductID 
left join Categories c on p.CategoryID = c.CategoryID 
and c.CategoryName = 'Confections' and year(o.OrderDate) = 1997
group by c2.CompanyName , c2.Phone , c2.CustomerID 
having count(c.CategoryName) = 0